#include "render_pass.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/swapchain.hpp"

namespace huedra {

void VulkanRenderPass::init(Device& device, const RenderPassBuilder& builder)
{
    p_device = &device;
    m_builder = builder;

    if (builder.getType() == RenderPassType::GRAPHICS)
    {
        for (auto& info : builder.getRenderTargets())
        {
            p_renderTargets.push_back(static_cast<VulkanRenderTarget*>(info.target.get()));
            p_renderTargets.back()->addRenderPass(this);
            if (p_renderTargets.back()->getSwapchain() != nullptr)
            {
                p_swapchainTarget = p_renderTargets.back();
            }
        }

        createRenderPass();
        switch (builder.getType())
        {
        case RenderPassType::GRAPHICS:
            m_pipeline.initGraphics(builder.getPipeline(), device, m_renderPass);
            break;
        case RenderPassType::COMPUTE:
            m_pipeline.initCompute(builder.getPipeline(), device);
            break;
        }
        createFramebuffers();
    }
    else
    {
        switch (builder.getType())
        {
        case RenderPassType::GRAPHICS:
            m_pipeline.initGraphics(builder.getPipeline(), device, m_renderPass);
            break;
        case RenderPassType::COMPUTE:
            m_pipeline.initCompute(builder.getPipeline(), device);
            break;
        }
    }
}

void VulkanRenderPass::cleanup()
{
    if (m_builder.getType() == RenderPassType::GRAPHICS)
    {
        cleanupFramebuffers();
        vkDestroyRenderPass(p_device->getLogical(), m_renderPass, nullptr);
        for (auto& renderTarget : p_renderTargets)
        {
            renderTarget->removeRenderPass(this);
        }
        p_swapchainTarget = nullptr;
    }
    m_pipeline.cleanup();
}

void VulkanRenderPass::createFramebuffers()
{
    if (p_swapchainTarget)
    {
        m_framebuffers.resize(p_swapchainTarget->getImageCount());
    }
    else
    {
        m_framebuffers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }
    for (size_t i = 0; i < m_framebuffers.size(); i++)
    {
        std::vector<VkImageView> attachments;
        for (auto& renderTarget : p_renderTargets)
        {
            u32 imageCount = renderTarget->getImageCount();
            if (renderTarget->usesColor())
            {
                attachments.push_back(renderTarget->getColorTexture().getView(i % imageCount));
            }
            if (renderTarget->usesDepth())
            {
                attachments.push_back(renderTarget->getDepthTexture().getView(i % imageCount));
            }
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = p_renderTargets[0]->getExtent().width;
        framebufferInfo.height = p_renderTargets[0]->getExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(p_device->getLogical(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create render target framebuffer!");
        }
    }
}

void VulkanRenderPass::cleanupFramebuffers()
{
    for (auto& framebuffer : m_framebuffers)
    {
        vkDestroyFramebuffer(p_device->getLogical(), framebuffer, nullptr);
    }
    m_framebuffers.clear();
}

void VulkanRenderPass::begin(VkCommandBuffer commandBuffer)
{
    if (m_builder.getType() == RenderPassType::COMPUTE)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.get());
        return;
    }

    VkExtent2D extent = p_renderTargets[0]->getExtent();

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(extent.height);
    viewport.width = static_cast<float>(extent.width);
    viewport.height = -static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    if (p_swapchainTarget)
    {
        renderPassInfo.framebuffer = m_framebuffers[p_swapchainTarget->getSwapchain()->getImageIndex()];
    }
    else
    {
        renderPassInfo.framebuffer = m_framebuffers[Global::graphicsManager.getCurrentFrame()];
    }
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    std::vector<VkClearValue> clearValues{};
    if (m_builder.getClearRenderTargets())
    {
        for (u32 i = 0; i < p_renderTargets.size(); ++i)
        {
            vec3 clearColor = m_builder.getRenderTargets()[i].clearColor;
            if (p_renderTargets[i]->usesColor())
            {
                VkClearValue& value = clearValues.emplace_back();
                value.color = {clearColor[0], clearColor[1], clearColor[2], 1.0f};
            }
            if (p_renderTargets[i]->usesDepth())
            {
                VkClearValue& value = clearValues.emplace_back();
                value.depthStencil = {1.0f, 0};
            }
        }
        renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    }

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.get());
}

void VulkanRenderPass::end(VkCommandBuffer commandBuffer)
{
    if (m_builder.getType() == RenderPassType::GRAPHICS)
    {
        vkCmdEndRenderPass(commandBuffer);
    }
}

void VulkanRenderPass::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachmentRef;
    std::vector<VkAttachmentReference> depthAttachmentRef;
    VkAttachmentLoadOp loadOp =
        m_builder.getClearRenderTargets() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    for (auto& renderTarget : p_renderTargets)
    {
        if (renderTarget->usesColor())
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = renderTarget->getColorFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = loadOp;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            attachments.push_back(colorAttachment);

            VkAttachmentReference attachmentRef{};
            attachmentRef.attachment = attachments.size() - 1;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorAttachmentRef.push_back(attachmentRef);
        }

        if (renderTarget->usesDepth())
        {
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = renderTarget->getDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = loadOp;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            attachments.push_back(depthAttachment);

            VkAttachmentReference attachmentRef{};
            attachmentRef.attachment = attachments.size() - 1;
            attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthAttachmentRef.push_back(attachmentRef);
        }
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<u32>(colorAttachmentRef.size());
    subpass.pColorAttachments = colorAttachmentRef.data();
    subpass.pDepthStencilAttachment = depthAttachmentRef.data();

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.srcAccessMask = 0;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    if (p_renderTargets[0]->usesColor())
    {
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
    }

    if (vkCreateRenderPass(p_device->getLogical(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create render pass!");
    }
}

} // namespace huedra