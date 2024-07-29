#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/config.hpp"

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

    VulkanSurfaceSupport surfaceSupport = m_device.querySurfaceSupport(m_device.getPhysical(), surface);
    VkSurfaceFormatKHR surfaceFormat = m_device.chooseSurfaceFormat(surfaceSupport.formats);
    m_renderPass = createRenderPass(surfaceFormat.format);

    vkDestroySurfaceKHR(m_instance.get(), surface, nullptr);
    tempWindow->cleanup();
    delete tempWindow;
}

void VulkanContext::cleanup()
{
    for (auto& swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }

    for (auto& buffer : m_buffers)
    {
        buffer->cleanup();
        delete buffer;
    }

    for (auto& resourceSet : m_resourceSets)
    {
        resourceSet->cleanup();
        delete resourceSet;
    }

    for (auto& pipeline : m_pipelines)
    {
        pipeline->cleanup();
        delete pipeline;
    }

    vkDestroyRenderPass(m_device.getLogical(), m_renderPass, nullptr);

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

void VulkanContext::createSwapchain(Window* window)
{
    VulkanSwapchain* swapchain = new VulkanSwapchain();
    VkSurfaceKHR surface = createSurface(window);
    swapchain->init(window, m_device, m_commandPool, surface, m_renderPass);

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

Pipeline* VulkanContext::createPipeline(const PipelineBuilder& pipelineBuilder)
{
    VulkanPipeline* pipeline = new VulkanPipeline();
    pipeline->initGraphics(pipelineBuilder, m_device, m_renderPass);
    m_pipelines.push_back(pipeline);
    return pipeline;
}

Buffer* VulkanContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data)
{
    VulkanBuffer* buffer = new VulkanBuffer();
    VkBufferUsageFlagBits bufferUsage = convertBufferUsage(usage);

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

ResourceSet* VulkanContext::createResourceSet(Pipeline* pipeline, u32 setIndex)
{
    VulkanResourceSet* resourceSet = new VulkanResourceSet();
    resourceSet->init(m_device, *static_cast<VulkanPipeline*>(pipeline), setIndex);
    m_resourceSets.push_back(resourceSet);
    return resourceSet;
}

void VulkanContext::submitGraphicsQueue()
{
    if (!m_recordedCommands)
    {
        return;
    }

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
    if (!m_recordedCommands)
    {
        return;
    }

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

void VulkanContext::prepareRendering()
{
    vkWaitForFences(m_device.getLogical(), 1, &m_renderingInFlightFences[Global::graphicsManager.getCurrentFrame()],
                    VK_TRUE, UINT64_MAX);
    m_recordedCommands = false;
}

void VulkanContext::recordGraphicsCommands(RenderPass& renderPass)
{
    if (!m_recordedCommands && renderPass.getRenderTarget().valid())
    {
        vkResetFences(m_device.getLogical(), 1, &m_renderingInFlightFences[Global::graphicsManager.getCurrentFrame()]);
        m_commandBuffer.begin(Global::graphicsManager.getCurrentFrame());
        m_recordedCommands = true;
    }
    recordCommandBuffer(m_commandBuffer.get(Global::graphicsManager.getCurrentFrame()), renderPass);
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

VkRenderPass VulkanContext::createRenderPass(VkFormat format)
{
    VkRenderPass renderPass;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Msaa
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = nullptr;
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

    std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};
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

VkBufferUsageFlagBits VulkanContext::convertBufferUsage(BufferUsageFlags usage)
{
    u32 result = 0;

    if (usage & HU_BUFFER_USAGE_VERTEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_INDEX_BUFFER)
    {
        result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_UNIFORM_BUFFER)
    {
        result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (usage & HU_BUFFER_USAGE_STORAGE_BUFFER)
    {
        result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    return static_cast<VkBufferUsageFlagBits>(result);
}

void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, RenderPass& renderPass)
{
    VulkanPipeline* pipeline = static_cast<VulkanPipeline*>(renderPass.getPipeline().get());
    VulkanRenderTarget* renderTarget = static_cast<VulkanRenderTarget*>(renderPass.getRenderTarget().get());

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = renderTarget->getFramebuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = renderTarget->getExtent();

    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};

    renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    VkExtent2D extent = renderTarget->getExtent();
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = static_cast<float>(extent.width);
    m_viewport.height = static_cast<float>(extent.height);
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissor.offset = {0, 0};
    m_scissor.extent = extent;

    vkCmdSetViewport(commandBuffer, 0, 1, &m_viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &m_scissor);

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    /*vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get());

    std::array<VkBuffer, 2> vertexBuffers{{m_vertexPositionsBuffer->get(), m_vertexColorsBuffer->get()}};
    std::array<VkDeviceSize, 2> offsets{{0, 0}};

    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);*/

    std::function<void(RenderContext&)> commands = pipeline->getBuilder().getRenderCommands();
    if (commands)
    {
        VulkanRenderContext renderContext;
        renderContext.init(commandBuffer, pipeline);
        commands(renderContext);
    }
    else
    {
        log(LogLevel::WARNING, "Could not execute render commands, not defined");
    }

    vkCmdEndRenderPass(commandBuffer);
}

} // namespace huedra