#include "render_target.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderTarget::init(Device& device, CommandPool& commandPool, VulkanSwapchain& swapchain, VkFormat format,
                              VkExtent2D extent, VkRenderPass renderPass)
{
    RenderTarget::init(swapchain.renderDepth() ? RenderTargetType::COLOR_AND_DEPTH : RenderTargetType::COLOR,
                       converter::convertVkFormat(format), extent.width, extent.height);
    p_device = &device;
    p_swapchain = &swapchain;
    m_extent = extent;

    u32 imageCount;
    vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &imageCount, images.data());

    m_texture.init(device, commandPool, images, format, extent);

    if (swapchain.renderDepth())
    {
        m_depthTexture.init(device, commandPool, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, extent.width,
                            extent.height, imageCount);
    }

    m_framebuffers.resize(imageCount);
    createFramebuffers(renderPass);
}

void VulkanRenderTarget::init(Device& device, CommandPool& commandPool, RenderTargetType type,
                              GraphicsDataFormat format, u32 width, u32 height, VkRenderPass renderPass)
{
    RenderTarget::init(type, format, width, height);
    p_device = &device;
    p_swapchain = nullptr;
    m_extent = {width, height};

    if (type == RenderTargetType::COLOR || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_texture.init(device, commandPool, TextureType::COLOR, format, width, height,
                       GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }
    if (type == RenderTargetType::DEPTH || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_depthTexture.init(device, commandPool, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, width, height,
                            GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    }

    m_framebuffers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    createFramebuffers(renderPass);
}

void VulkanRenderTarget::cleanup()
{
    for (size_t i = 0; i < m_framebuffers.size(); ++i)
    {
        vkDestroyFramebuffer(p_device->getLogical(), m_framebuffers[i], nullptr);
    }
    if (getType() == RenderTargetType::COLOR || getType() == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_texture.cleanup();
    }
    if (getType() == RenderTargetType::DEPTH || getType() == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_depthTexture.cleanup();
    }
}

void VulkanRenderTarget::prepareNextFrame(u32 frameIndex)
{
    if (p_swapchain)
    {
        std::optional index = p_swapchain->aquireNextImage(frameIndex);
        if (index.has_value())
        {
            m_currentImageIndex = index.value();
        }
        setAvailability(index.has_value());
    }
    else
    {
        m_currentImageIndex = Global::graphicsManager.getCurrentFrame();
        setAvailability(true);
    }
}

void VulkanRenderTarget::createFramebuffers(VkRenderPass renderPass)
{
    for (size_t i = 0; i < m_framebuffers.size(); i++)
    {
        std::vector<VkImageView> attachments;
        if (getType() == RenderTargetType::COLOR || getType() == RenderTargetType::COLOR_AND_DEPTH)
        {
            attachments.push_back(m_texture.getView(i));
        }
        if (getType() == RenderTargetType::DEPTH || getType() == RenderTargetType::COLOR_AND_DEPTH)
        {
            attachments.push_back(m_depthTexture.getView(i));
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_extent.width;
        framebufferInfo.height = m_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(p_device->getLogical(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create render target framebuffer!");
        }
    }
}

} // namespace huedra
