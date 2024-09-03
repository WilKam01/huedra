#include "render_pass.hpp"
#include "core/global.hpp"
#include "core/log.hpp"

namespace huedra {

void VulkanRenderPass::init(Device& device, const PipelineBuilder& builder, RenderCommands commands,
                            VulkanRenderTarget* renderTarget, RenderPassSettings settings)
{
    p_device = &device;
    m_commands = commands;
    p_renderTarget = Ref<RenderTarget>(renderTarget);
    p_vkRenderTarget = renderTarget;
    m_settings = settings;

    renderTarget->addRenderPass(this);
    createRenderPass();
    m_pipeline.initGraphics(builder, device, m_renderPass);
    createFramebuffers();
}

void VulkanRenderPass::cleanup()
{
    cleanupFramebuffers();
    m_pipeline.cleanup();
    vkDestroyRenderPass(p_device->getLogical(), m_renderPass, nullptr);
}

void VulkanRenderPass::createFramebuffers()
{
    m_framebuffers.resize(p_vkRenderTarget->getImageCount());
    for (size_t i = 0; i < m_framebuffers.size(); i++)
    {
        std::vector<VkImageView> attachments;
        if (p_vkRenderTarget->usesColor())
        {
            attachments.push_back(p_vkRenderTarget->getColorTexture().getView(i));
        }
        if (p_vkRenderTarget->usesDepth())
        {
            attachments.push_back(p_vkRenderTarget->getDepthTexture().getView(i));
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = p_vkRenderTarget->getExtent().width;
        framebufferInfo.height = p_vkRenderTarget->getExtent().height;
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
    VkExtent2D extent = p_vkRenderTarget->getExtent();

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
    renderPassInfo.framebuffer = m_framebuffers[p_vkRenderTarget->getImageIndex()];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    std::vector<VkClearValue> clearValues{};
    if (m_settings.clearRenderTarget)
    {
        if (p_vkRenderTarget->usesColor())
        {
            VkClearValue& value = clearValues.emplace_back();
            value.color = {m_settings.clearColor[0], m_settings.clearColor[1], m_settings.clearColor[2], 1.0f};
        }
        if (p_vkRenderTarget->usesDepth())
        {
            VkClearValue& value = clearValues.emplace_back();
            value.depthStencil = {1.0f, 0};
        }
        renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    }

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.get());
}

void VulkanRenderPass::end(VkCommandBuffer commandBuffer) { vkCmdEndRenderPass(commandBuffer); }

void VulkanRenderPass::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    RenderTargetType type = p_vkRenderTarget->getType();
    if (p_vkRenderTarget->usesColor())
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = p_vkRenderTarget->getColorFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp =
            m_settings.clearRenderTarget ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        attachments.push_back(colorAttachment);
    }

    if (p_vkRenderTarget->usesDepth())
    {
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = p_vkRenderTarget->getDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp =
            m_settings.clearRenderTarget ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments.push_back(depthAttachment);
    }

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = nullptr;
    subpass.pResolveAttachments = nullptr;

    if (type == RenderTargetType::COLOR || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
    }
    if (type == RenderTargetType::DEPTH || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
    }

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
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(p_device->getLogical(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create render pass!");
    }
}

} // namespace huedra