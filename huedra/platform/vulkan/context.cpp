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

    m_graphicsCommandPool.init(m_device, VK_PIPELINE_BIND_POINT_GRAPHICS);
    m_computeCommandPool.init(m_device, VK_PIPELINE_BIND_POINT_COMPUTE);

    m_graphicsCommandBuffer.init(m_device, m_graphicsCommandPool, GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeCommandBuffer.init(m_device, m_computeCommandPool, GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    m_graphicsFrameInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeFrameInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[0].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[1].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeSyncSemaphores[0].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeSyncSemaphores[1].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_graphicsFrameInFlightFences[i]) !=
                VK_SUCCESS ||
            vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_computeFrameInFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[0][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[1][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_computeSyncSemaphores[0][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_computeSyncSemaphores[1][i]) !=
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

    for (auto& texture : m_textures)
    {
        texture.cleanup();
    }
    m_textures.clear();

    for (auto& renderTarget : m_renderTargets)
    {
        renderTarget.cleanup();
    }
    m_renderTargets.clear();

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
    m_usingGraphicsQueue = false;
    m_usingComputeQueue = false;

    for (auto& swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }
    m_swapchains.clear();

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device.getLogical(), m_computeSyncSemaphores[0][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_computeSyncSemaphores[1][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[0][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[1][i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_computeFrameInFlightFences[i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_graphicsFrameInFlightFences[i], nullptr);
    }

    m_computeCommandBuffer.cleanup();
    m_graphicsCommandBuffer.cleanup();

    m_computeCommandPool.cleanup();
    m_graphicsCommandPool.cleanup();

    m_device.cleanup();
    m_instance.cleanup();
}

void VulkanContext::createSwapchain(Window* window, bool renderDepth)
{
    VulkanSwapchain* swapchain = new VulkanSwapchain();
    VkSurfaceKHR surface = createSurface(window);
    swapchain->init(window, m_device, m_graphicsCommandPool, surface, renderDepth);

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
        m_stagingBuffer.init(m_device, BufferType::STATIC, size, HU_BUFFER_USAGE_UNDEFINED,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);

        buffer.init(m_device, type, size, usage, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = m_graphicsCommandPool.beginSingleTimeCommand();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(commandBuffer, m_stagingBuffer.get(), buffer.get(), 1, &copyRegion);
        m_graphicsCommandPool.endSingleTimeCommand(commandBuffer);

        m_stagingBuffer.cleanup();
    }
    else
    {
        buffer.init(m_device, type, size, usage, bufferUsage,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);
    }

    return &buffer;
}

Texture* VulkanContext::createTexture(TextureData textureData)
{
    VulkanTexture& texture = m_textures.emplace_back();
    texture.init(m_device, m_graphicsCommandPool, textureData);
    return &texture;
}

RenderTarget* VulkanContext::createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height)
{
    VulkanRenderTarget& renderTarget = m_renderTargets.emplace_back();
    renderTarget.init(m_device, m_graphicsCommandPool, type, format, width, height);
    return &renderTarget;
}

void VulkanContext::removeBuffer(Buffer* buffer)
{
    auto it =
        std::find_if(m_buffers.begin(), m_buffers.end(), [&](VulkanBuffer& vkBuffer) { return &vkBuffer == buffer; });
    if (it != m_buffers.end())
    {
        it->cleanup();
        m_buffers.erase(it);
    }
}

void VulkanContext::removeTexture(Texture* texture)
{
    auto it = std::find_if(m_textures.begin(), m_textures.end(),
                           [&](VulkanTexture& vkTexture) { return &vkTexture == texture; });
    if (it != m_textures.end())
    {
        it->cleanup();
        m_textures.erase(it);
    }
}

void VulkanContext::removeRenderTarget(RenderTarget* renderTarget)
{
    auto it = std::find_if(m_renderTargets.begin(), m_renderTargets.end(),
                           [&](VulkanRenderTarget& vkRenderTarget) { return &vkRenderTarget == renderTarget; });
    if (it != m_renderTargets.end())
    {
        it->cleanup();
        m_renderTargets.erase(it);
    }
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
    m_usingGraphicsQueue = false;
    m_usingComputeQueue = false;

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

        switch (info.getType())
        {
        case RenderPassType::GRAPHICS:
            m_passBatches.back().useGraphicsQueue = true;
            m_usingGraphicsQueue = true;
            break;
        case RenderPassType::COMPUTE:
            m_passBatches.back().useComputeQueue = true;
            m_usingComputeQueue = true;
            break;
        }

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
    m_curGraphicsSemphoreIndex = 0;
    m_curComputeSemphoreIndex = 0;

    std::vector<VkFence> fences{};
    if (m_usingGraphicsQueue)
    {
        fences.push_back(m_graphicsFrameInFlightFences[Global::graphicsManager.getCurrentFrame()]);
    }
    if (m_usingComputeQueue)
    {
        fences.push_back(m_computeFrameInFlightFences[Global::graphicsManager.getCurrentFrame()]);
    }
    vkWaitForFences(m_device.getLogical(), static_cast<u32>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);
    vkResetFences(m_device.getLogical(), static_cast<u32>(fences.size()), fences.data());

    for (u32 i = 0; i < m_passBatches.size(); ++i)
    {
        if (m_passBatches[i].useGraphicsQueue)
        {
            m_graphicsCommandBuffer.begin(Global::graphicsManager.getCurrentFrame());
        }
        if (m_passBatches[i].useComputeQueue)
        {
            m_computeCommandBuffer.begin(Global::graphicsManager.getCurrentFrame());
        }

        for (auto& info : m_passBatches[i].passes)
        {
            VkCommandBuffer commandBuffer;
            switch (info.pass->getPipelineType())
            {
            case PipelineType::GRAPHICS:
                commandBuffer = m_graphicsCommandBuffer.get(Global::graphicsManager.getCurrentFrame());
                break;
            case PipelineType::COMPUTE:
                commandBuffer = m_computeCommandBuffer.get(Global::graphicsManager.getCurrentFrame());
                break;
            }

            info.pass->begin(commandBuffer);

            VulkanRenderContext renderContext;
            renderContext.init(commandBuffer, this, info.pass,
                               info.descriptorHandlers[Global::graphicsManager.getCurrentFrame()]);
            info.pass->getCommands()(renderContext);

            info.pass->end(commandBuffer);
        }

        if (m_passBatches[i].useGraphicsQueue)
        {
            submitGraphicsQueue(i);
        }
        if (m_passBatches[i].useComputeQueue)
        {
            submitComputeQueue(i);
        }
    }

    if (!m_activeSwapchains.empty())
    {
        presentSwapchains();
    }
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
    m_graphicsCommandBuffer.end(Global::graphicsManager.getCurrentFrame());

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
        if (m_passBatches[batchIndex - 1].useGraphicsQueue)
        {
            waitSemaphores.push_back(
                m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][Global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
        if (m_passBatches[batchIndex - 1].useComputeQueue)
        {
            waitSemaphores.push_back(
                m_computeSyncSemaphores[1 - m_curComputeSemphoreIndex][Global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_graphicsCommandBuffer.get(Global::graphicsManager.getCurrentFrame());

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
        fence = m_graphicsFrameInFlightFences[Global::graphicsManager.getCurrentFrame()];
    }
    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit graphics queue!");
    }

    m_curGraphicsSemphoreIndex = 1 - m_curGraphicsSemphoreIndex;
}

void VulkanContext::submitComputeQueue(u32 batchIndex)
{
    m_computeCommandBuffer.end(Global::graphicsManager.getCurrentFrame());

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    if (batchIndex != 0) // Should wait on queue if it's not the first batch
    {
        if (m_passBatches[batchIndex - 1].useGraphicsQueue)
        {
            waitSemaphores.push_back(
                m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][Global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
        if (m_passBatches[batchIndex - 1].useComputeQueue)
        {
            waitSemaphores.push_back(
                m_computeSyncSemaphores[1 - m_curComputeSemphoreIndex][Global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_computeCommandBuffer.get(Global::graphicsManager.getCurrentFrame());

    // Should not signal last batch
    if (batchIndex != m_passBatches.size() - 1)
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores =
            &m_computeSyncSemaphores[m_curComputeSemphoreIndex][Global::graphicsManager.getCurrentFrame()];
    }

    VkFence fence{};
    if (batchIndex == m_passBatches.size() - 1)
    {
        fence = m_computeFrameInFlightFences[Global::graphicsManager.getCurrentFrame()];
    }
    if (vkQueueSubmit(m_device.getComputeQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit compute queue!");
    }

    m_curComputeSemphoreIndex = 1 - m_curComputeSemphoreIndex;
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