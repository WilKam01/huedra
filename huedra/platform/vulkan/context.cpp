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

    m_renderingInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_renderFinishedSemaphores.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_renderingInFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
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
        buffer->cleanup();
        delete buffer;
    }
    m_buffers.clear();

    for (auto& texture : m_textures)
    {
        texture->cleanup();
        delete texture;
    }
    m_textures.clear();

    for (auto& [name, renderPass] : m_renderPasses)
    {
        renderPass.pass->cleanup();
        delete renderPass.pass;
        for (auto& handler : renderPass.descriptorHandlers)
        {
            handler.cleanup();
        }
        vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
    }
    m_renderPasses.clear();

    for (auto& swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }
    m_swapchains.clear();

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device.getLogical(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_renderingInFlightFences[i], nullptr);
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
    VulkanBuffer* buffer = new VulkanBuffer();
    VkBufferUsageFlagBits bufferUsage = converter::convertBufferUsage(usage);

    if (type == BufferType::STATIC && data)
    {
        m_stagingBuffer.init(m_device, BufferType::STATIC, HU_BUFFER_USAGE_UNDEFINED, size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);

        buffer->init(m_device, type, usage, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = m_commandPool.beginSingleTimeCommand();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(commandBuffer, m_stagingBuffer.get(), buffer->get(), 1, &copyRegion);
        m_commandPool.endSingleTimeCommand(commandBuffer);

        m_stagingBuffer.cleanup();
    }
    else
    {
        buffer->init(m_device, type, usage, size, bufferUsage,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);
    }

    m_buffers.push_back(buffer);
    return buffer;
}

Texture* VulkanContext::createTexture(TextureData textureData)
{
    VulkanTexture* texture = new VulkanTexture();
    texture->init(m_device, m_commandPool, textureData);
    m_textures.push_back(texture);
    return texture;
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

    // Destroy all previous passes
    for (auto& [name, renderPass] : m_renderPasses)
    {
        renderPass.pass->cleanup();
        delete renderPass.pass;
        for (auto& handler : renderPass.descriptorHandlers)
        {
            handler.cleanup();
        }
        vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
    }
    m_renderPasses.clear();

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

        m_renderPasses.insert(std::pair<std::string, PassInfo>(key, passInfo));
    }
}

void VulkanContext::render()
{
    vkWaitForFences(m_device.getLogical(), 1, &m_renderingInFlightFences[Global::graphicsManager.getCurrentFrame()],
                    VK_TRUE, UINT64_MAX);
    m_recordedCommands = false;

    for (auto& [name, info] : m_renderPasses)
    {
        if (!info.pass->getRenderTarget().valid())
        {
            continue;
        }

        RenderTarget* renderTarget = info.pass->getRenderTarget().get();
        renderTarget->prepareNextFrame(Global::graphicsManager.getCurrentFrame());
        if (renderTarget->isAvailable())
        {
            if (!m_recordedCommands)
            {
                vkResetFences(m_device.getLogical(), 1,
                              &m_renderingInFlightFences[Global::graphicsManager.getCurrentFrame()]);
                m_commandBuffer.begin(Global::graphicsManager.getCurrentFrame());
                m_recordedCommands = true;
            }

            VkCommandBuffer commandBuffer = m_commandBuffer.get(Global::graphicsManager.getCurrentFrame());
            info.pass->begin(commandBuffer);

            VulkanRenderContext renderContext;
            renderContext.init(commandBuffer, info.pass,
                               info.descriptorHandlers[Global::graphicsManager.getCurrentFrame()]);
            info.pass->getCommands()(renderContext);

            info.pass->end(commandBuffer);
        }
    }

    if (m_recordedCommands)
    {
        submitGraphicsQueue();
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

VkRenderPass VulkanContext::createRenderPass(VkFormat format, VkFormat depthFormat)
{
    VkRenderPass renderPass;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = nullptr;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.srcAccessMask = 0;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_device.getLogical(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create render pass!");
    }

    return renderPass;
}

void VulkanContext::submitGraphicsQueue()
{
    m_commandBuffer.end(Global::graphicsManager.getCurrentFrame());

    std::vector<VkSemaphore> waitSemaphores;
    for (auto& swapchain : m_swapchains)
    {
        if (swapchain->canPresent())
        {
            waitSemaphores.push_back(swapchain->getImageAvailableSemaphore(Global::graphicsManager.getCurrentFrame()));
        }
    }

    if (waitSemaphores.empty())
    {
        return;
    }

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer.get(Global::graphicsManager.getCurrentFrame());

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[Global::graphicsManager.getCurrentFrame()]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo,
                      m_renderingInFlightFences[Global::graphicsManager.getCurrentFrame()]) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit draw command buffer!");
    }
}

void VulkanContext::presentSwapchains()
{
    VkSemaphore waitSemaphores[] = {m_renderFinishedSemaphores[Global::graphicsManager.getCurrentFrame()]};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphores;

    std::vector<VkSwapchainKHR> swapchains;
    std::vector<u32> imageIndices;
    std::vector<VkResult> results;
    for (auto& swapchain : m_swapchains)
    {
        if (swapchain->canPresent())
        {
            swapchains.push_back(swapchain->get());
            imageIndices.push_back(swapchain->getRenderTarget().getImageIndex());
            results.push_back(VK_SUCCESS);
        }
    }

    if (swapchains.empty())
    {
        return;
    }

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
    for (auto& swapchain : m_swapchains)
    {
        if (swapchain->canPresent())
        {
            swapchain->handlePresentResult(results[index++]);
        }
    }
}

} // namespace huedra