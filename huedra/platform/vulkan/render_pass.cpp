#include "render_pass.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/render_target.hpp"
#include "platform/vulkan/swapchain.hpp"

namespace huedra {

void VulkanRenderPass::init(Device& device, const RenderPassBuilder& builder)
{
    p_device = &device;
    m_builder = builder;
    if (m_builder.getType() == RenderPassType::GRAPHICS)
    {
        for (auto& info : m_builder.getRenderTargets())
        {
            p_targetInfos.push_back({static_cast<VulkanRenderTarget*>(info.target.get())});
            p_targetInfos.back().renderTarget->addRenderPass(this);
            if (p_targetInfos.back().renderTarget->getSwapchain() != nullptr)
            {
                p_swapchainTarget = p_targetInfos.back().renderTarget;
            }
        }
    }
}

void VulkanRenderPass::create()
{
    if (m_builder.getType() == RenderPassType::GRAPHICS)
    {
        createRenderPass();
        m_pipeline.initGraphics(m_builder.getPipeline(), *p_device, m_renderPass,
                                static_cast<u32>(m_builder.getRenderTargets().size()));
        createFramebuffers();
    }
    else
    {
        m_pipeline.initCompute(m_builder.getPipeline(), *p_device);
    }
}

void VulkanRenderPass::cleanup()
{
    if (m_builder.getType() == RenderPassType::GRAPHICS)
    {
        cleanupFramebuffers();
        vkDestroyRenderPass(p_device->getLogical(), m_renderPass, nullptr);
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
        for (auto& info : p_targetInfos)
        {
            u32 imageCount = info.renderTarget->getImageCount();
            if (info.renderTarget->usesColor())
            {
                attachments.push_back(info.renderTarget->getVkColorTexture().getView(i % imageCount));
            }
            if (info.renderTarget->usesDepth())
            {
                attachments.push_back(info.renderTarget->getVkDepthTexture().getView(i % imageCount));
            }
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = p_targetInfos[0].renderTarget->getExtent().width;
        framebufferInfo.height = p_targetInfos[0].renderTarget->getExtent().height;
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

    VkExtent2D extent = p_targetInfos[0].renderTarget->getExtent();

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
        renderPassInfo.framebuffer = m_framebuffers[global::graphicsManager.getCurrentFrame()];
    }
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;
    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    std::vector<VkClearValue> clearValues{};
    if (m_builder.getClearRenderTargets())
    {
        for (u32 i = 0; i < p_targetInfos.size(); ++i)
        {
            vec3 clearColor = m_builder.getRenderTargets()[i].clearColor;
            if (p_targetInfos[i].renderTarget->usesColor())
            {
                VkClearValue& value = clearValues.emplace_back();
                value.color = {clearColor[0], clearColor[1], clearColor[2], 1.0f};
            }
            if (p_targetInfos[i].renderTarget->usesDepth())
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
        for (auto& info : p_targetInfos)
        {
            if (info.renderTarget->usesColor())
            {
                info.renderTarget->getVkColorTexture().setLayout(info.finalColorLayout);
            }
            if (info.renderTarget->usesDepth())
            {
                info.renderTarget->getVkDepthTexture().setLayout(info.finalDepthLayout);
            }
        }
    }
}

void VulkanRenderPass::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorAttachmentRef;
    std::vector<VkAttachmentReference> depthAttachmentRef;
    VkAttachmentLoadOp loadOp =
        m_builder.getClearRenderTargets() ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    for (auto& info : p_targetInfos)
    {
        if (info.renderTarget->usesColor())
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = info.renderTarget->getColorFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = loadOp;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = info.initialColorLayout;
            colorAttachment.finalLayout = info.finalColorLayout;

            attachments.push_back(colorAttachment);

            VkAttachmentReference attachmentRef{};
            attachmentRef.attachment = attachments.size() - 1;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorAttachmentRef.push_back(attachmentRef);
        }

        if (info.renderTarget->usesDepth())
        {
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = info.renderTarget->getDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = loadOp;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = info.initialDepthLayout;
            depthAttachment.finalLayout = info.finalDepthLayout;

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
    if (p_targetInfos[0].renderTarget->usesColor())
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