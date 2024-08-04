#include "render_target.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/render_pass.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderTarget::init(Device& device, CommandPool& commandPool, VulkanSwapchain& swapchain, VkFormat format,
                              VkExtent2D extent)
{
    RenderTarget::init(swapchain.renderDepth() ? RenderTargetType::COLOR_AND_DEPTH : RenderTargetType::COLOR,
                       converter::convertVkFormat(format), extent.width, extent.height);
    p_device = &device;
    p_swapchain = &swapchain;
    m_extent = extent;

    vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &m_imageCount, nullptr);
    std::vector<VkImage> images(m_imageCount);
    vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &m_imageCount, images.data());

    m_texture.init(device, commandPool, images, format, extent);

    if (swapchain.renderDepth())
    {
        m_depthTexture.init(device, commandPool, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, extent.width,
                            extent.height, m_imageCount);
    }

    for (auto& renderPass : p_renderPasses)
    {
        renderPass->createFramebuffers();
    }
    m_initialized = true;
}

void VulkanRenderTarget::init(Device& device, CommandPool& commandPool, RenderTargetType type,
                              GraphicsDataFormat format, u32 width, u32 height)
{
    RenderTarget::init(type, format, width, height);
    p_device = &device;
    p_swapchain = nullptr;
    m_extent = {width, height};
    m_imageCount = GraphicsManager::MAX_FRAMES_IN_FLIGHT;

    if (type == RenderTargetType::COLOR || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_texture.init(device, commandPool, TextureType::COLOR, format, width, height, m_imageCount);
    }
    if (type == RenderTargetType::DEPTH || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_depthTexture.init(device, commandPool, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, width, height,
                            m_imageCount);
    }

    for (auto& renderPass : p_renderPasses)
    {
        renderPass->createFramebuffers();
    }
    m_initialized = true;
}

void VulkanRenderTarget::cleanup()
{
    partialCleanup();
    p_renderPasses.clear();
}

void VulkanRenderTarget::partialCleanup()
{
    if (m_initialized)
    {
        for (auto& renderPass : p_renderPasses)
        {
            renderPass->cleanupFramebuffers();
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
    m_initialized = false;
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

void VulkanRenderTarget::addRenderPass(VulkanRenderPass* renderPass) { p_renderPasses.push_back(renderPass); }

} // namespace huedra
