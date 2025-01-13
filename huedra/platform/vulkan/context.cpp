#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/config.hpp"
#include "platform/vulkan/type_converter.hpp"

#ifdef WIN32
#include "platform/win32/window.hpp"
#endif

namespace huedra {

void VulkanContext::init()
{
    m_instance.init();

    Window* tempWindow = Global::windowManager.createWindow("temp", {});
    VkSurfaceKHR surface = createSurface(tempWindow);

    m_device.init(m_instance, surface);

    m_commandPool.init(m_device, VK_PIPELINE_BIND_POINT_GRAPHICS);
    m_commandBuffer.init(m_device, m_commandPool, GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    m_frameInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[0].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[1].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_frameInFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[0][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[1][i]) !=
                VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create render fences and semaphores!");
        }
    }

    vkDestroySurfaceKHR(m_instance.get(), surface, nullptr);
    tempWindow->cleanup();
    delete tempWindow;
}

void VulkanContext::cleanup()
{
    for (auto& buffer : m_buffers)
    {
        buffer.cleanup();
    }
    m_buffers.clear();
    m_bufferHandles.clear();

    for (auto& texture : m_textures)
    {
        texture.cleanup();
    }
    m_textures.clear();
    m_textureHandles.clear();

    for (auto& batch : m_passBatches)
    {
        for (auto& renderPass : batch.passes)
        {
            renderPass.pass->cleanup();
            delete renderPass.pass;
            for (auto& handler : renderPass.descriptorHandlers)
            {
                handler.cleanup();
            }
            vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
        }
    }
    m_passBatches.clear();
    m_activeSwapchains.clear();

    for (auto& swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }
    m_swapchains.clear();

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[0][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[1][i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_frameInFlightFences[i], nullptr);
    }

    m_commandBuffer.cleanup();
    m_commandPool.cleanup();
    m_device.cleanup();
    m_instance.cleanup();
}

void VulkanContext::createSwapchain(Window* window, bool renderDepth)
{
    VulkanSwapchain* swapchain = new VulkanSwapchain();
    VkSurfaceKHR surface = createSurface(window);
    swapchain->init(window, m_device, m_commandPool, surface, renderDepth);

    m_swapchains.push_back(swapchain);
    m_surfaces.push_back(surface);
}

void VulkanContext::removeSwapchain(size_t index)
{
    VulkanSwapchain* swapchain = m_swapchains[index];
    m_swapchains.erase(m_swapchains.begin() + index);

    swapchain->cleanup();
    delete swapchain;

    vkDestroySurfaceKHR(m_instance.get(), m_surfaces[index], nullptr);
    m_surfaces.erase(m_surfaces.begin() + index);
}

Buffer* VulkanContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data)
{
    VulkanBuffer& buffer = m_buffers.emplace_back();
    VkBufferUsageFlagBits bufferUsage = converter::convertBufferUsage(usage);

    if (type == BufferType::STATIC && data)
    {
        m_stagingBuffer.init(m_device, BufferType::STATIC, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);

        buffer.init(m_device, type, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = m_commandPool.beginSingleTimeCommand();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(commandBuffer, m_stagingBuffer.get(), buffer.get(), 1, &copyRegion);
        m_commandPool.endSingleTimeCommand(commandBuffer);

        m_stagingBuffer.cleanup();
    }
    else
    {
        buffer.init(m_device, type, size, bufferUsage,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);
    }

    Buffer& buf = m_bufferHandles.emplace_back();
    buf.init(type, usage, size, this);
    buf.setId(m_buffers.size() - 1);
    return &buf;
}

void VulkanContext::readBuffer(u64 id, u64 size, void* data)
{
    if (id >= m_buffers.size())
    {
        log(LogLevel::WARNING, "Could not read buffer, id: %llu invalid. Only %llu buffers exists", id,
            m_buffers.size());
        return;
    }

    m_buffers[id].read(size, data);
}

void VulkanContext::writeToBuffer(u64 id, u64 size, void* data)
{
    if (id >= m_buffers.size())
    {
        log(LogLevel::WARNING, "Could not write to buffer, id: %llu invalid. Only %llu buffers exists", id,
            m_buffers.size());
        return;
    }

    m_buffers[id].write(size, data);
}

Texture* VulkanContext::createTexture(TextureData textureData)
{
    VulkanTexture& texture = m_textures.emplace_back();
    texture.init(m_device, m_commandPool, textureData);

    Texture& tex = m_textureHandles.emplace_back();
    tex.init(textureData.width, textureData.height, textureData.format, TextureType::COLOR, this);
    tex.setId(m_textures.size() - 1);
    return &tex;
}

void VulkanContext::prepareSwapchains()
{
    for (auto& swapchain : m_swapchains)
    {
        swapchain->aquireNextImage();
    }
}

void VulkanContext::setRenderGraph(RenderGraphBuilder& builder)
{
    if (m_curGraph.getHash() == builder.getHash())
    {
        return;
    }

    m_curGraph = builder;
    m_device.waitIdle();

    log(LogLevel::INFO, "New render graph with hash: %llu", m_curGraph.getHash());

    // Destroy all previous batches
    for (auto& batch : m_passBatches)
    {
        for (auto& renderPass : batch.passes)
        {
            renderPass.pass->cleanup();
            delete renderPass.pass;
            for (auto& handler : renderPass.descriptorHandlers)
            {
                handler.cleanup();
            }
            vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
        }
    }
    m_passBatches.clear();
    m_activeSwapchains.clear();

    m_passBatches.emplace_back(); // At least one batch
    for (auto& [key, info] : builder.getRenderPasses())
    {
        PassInfo passInfo;
        passInfo.pass = new VulkanRenderPass();
        passInfo.pass->init(m_device, info);

        std::vector<std::vector<ResourceBinding>> sets = info.getPipeline().getResources();
        std::vector<std::vector<VkDescriptorType>> bindingTypes(sets.size());
        std::multiset<VkDescriptorType> poolSizeSet{};
        for (u64 i = 0; i < sets.size(); ++i)
        {
            bindingTypes[i].resize(sets[i].size());
            for (u64 j = 0; j < sets[i].size(); ++j)
            {
                bindingTypes[i][j] = converter::convertResourceType(sets[i][j].resource);
                poolSizeSet.insert(bindingTypes[i][j]);
            }
        }

        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& type : poolSizeSet)
        {
            VkDescriptorPoolSize poolSize;
            poolSize.type = type;
            // TODO: Add ability to decide multiple sets per frame. An estimation or actual?
            poolSize.descriptorCount =
                static_cast<u32>(poolSizeSet.count(type) * GraphicsManager::MAX_FRAMES_IN_FLIGHT);
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
        // TODO: Add ability to decide multiple sets per frame. An estimation or actual?
        poolInfo.maxSets = GraphicsManager::MAX_FRAMES_IN_FLIGHT;

        if (vkCreateDescriptorPool(m_device.getLogical(), &poolInfo, nullptr, &passInfo.descriptorPool) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create descriptor pool!");
        }

        passInfo.descriptorHandlers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        for (auto& handler : passInfo.descriptorHandlers)
        {
            handler.init(m_device, *passInfo.pass, passInfo.descriptorPool, bindingTypes);
        }

        m_passBatches.back().passes.push_back(passInfo);

        for (auto& target : info.getRenderTargets())
        {
            VulkanRenderTarget* vulkanTarget = static_cast<VulkanRenderTarget*>(target.target.get());
            if (vulkanTarget->getSwapchain() != nullptr)
            {
                m_passBatches.back().swapchains.insert(vulkanTarget->getSwapchain());
                m_activeSwapchains.insert(vulkanTarget->getSwapchain());
            }
        }
    }
}

void VulkanContext::render()
{
    vkWaitForFences(m_device.getLogical(), 1, &m_frameInFlightFences[Global::graphicsManager.getCurrentFrame()],
                    VK_TRUE, UINT64_MAX);
    m_curGraphicsSemphoreIndex = 0;

    vkResetFences(m_device.getLogical(), 1, &m_frameInFlightFences[Global::graphicsManager.getCurrentFrame()]);
    m_commandBuffer.begin(Global::graphicsManager.getCurrentFrame());

    for (u32 i = 0; i < m_passBatches.size(); ++i)
    {
        for (auto& info : m_passBatches[i].passes)
        {
            VkCommandBuffer commandBuffer = m_commandBuffer.get(Global::graphicsManager.getCurrentFrame());
            info.pass->begin(commandBuffer);

            VulkanRenderContext renderContext;
            renderContext.init(commandBuffer, this, info.pass,
                               info.descriptorHandlers[Global::graphicsManager.getCurrentFrame()]);
            info.pass->getCommands()(renderContext);

            info.pass->end(commandBuffer);
        }

        submitGraphicsQueue(i);
    }

    if (!m_activeSwapchains.empty())
    {
        presentSwapchains();
    }
}

VulkanBuffer* VulkanContext::getBuffer(u64 id)
{
    if (id >= m_buffers.size())
    {
        log(LogLevel::WARNING, "Could not get buffer, id: %llu invalid. Only %llu buffers exists", id,
            m_buffers.size());
        return nullptr;
    }
    return &m_buffers[id];
}

VulkanTexture* VulkanContext::getTexture(u64 id)
{
    if (id >= m_textures.size())
    {
        log(LogLevel::WARNING, "Could not get texture, id: %llu invalid. Only %llu textures exists", id,
            m_textures.size());
        return nullptr;
    }
    return &m_textures[id];
}

VkSurfaceKHR VulkanContext::createSurface(Window* window)
{
    VkSurfaceKHR surface;

#ifdef WIN32
    Win32Window* win = static_cast<Win32Window*>(window);
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(NULL);
    createInfo.hwnd = win->getHandle();

    if (vkCreateWin32SurfaceKHR(m_instance.get(), &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create Win32 surface!");
    }
#endif

    return surface;
}

void VulkanContext::submitGraphicsQueue(u32 batchIndex)
{
    m_commandBuffer.end(Global::graphicsManager.getCurrentFrame());

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;

    for (auto& swapchain : m_passBatches[batchIndex].swapchains)
    {
        waitSemaphores.push_back(swapchain->getImageAvailableSemaphore());
        waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    if (batchIndex != 0) // Should wait on queue if it's not the first batch
    {
        waitSemaphores.push_back(
            m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][Global::graphicsManager.getCurrentFrame()]);
        waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer.get(Global::graphicsManager.getCurrentFrame());

    // Should not signal last batch, EXCEPT for existence of active swapchains
    if (batchIndex != m_passBatches.size() - 1 || !m_activeSwapchains.empty())
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores =
            &m_graphicsSyncSemaphores[m_curGraphicsSemphoreIndex][Global::graphicsManager.getCurrentFrame()];
    }

    VkFence fence{};
    if (batchIndex == m_passBatches.size() - 1)
    {
        fence = m_frameInFlightFences[Global::graphicsManager.getCurrentFrame()];
    }
    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit graphics queue!");
    }

    m_curGraphicsSemphoreIndex = 1 - m_curGraphicsSemphoreIndex;
}

void VulkanContext::presentSwapchains()
{
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<u32> imageIndices;
    std::vector<VkResult> results;
    for (auto& swapchain : m_activeSwapchains)
    {
        swapchains.push_back(swapchain->get());
        imageIndices.push_back(swapchain->getImageIndex());
        results.push_back(VK_SUCCESS);
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores =
        &m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][Global::graphicsManager.getCurrentFrame()];
    presentInfo.swapchainCount = static_cast<u32>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();
    presentInfo.pResults = results.data();

    VkResult result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)
    {
        log(LogLevel::ERR, "Failed to present swap chain images!");
    }

    u32 index = 0;
    for (auto& swapchain : m_activeSwapchains)
    {
        swapchain->handlePresentResult(results[index++]);
    }
}

} // namespace huedra