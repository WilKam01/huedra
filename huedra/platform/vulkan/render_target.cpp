#include "render_target.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/render_pass.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanRenderTarget::init(Device& device, VulkanSwapchain& swapchain, VkFormat format, VkExtent2D extent)
{
    RenderTarget::init(swapchain.renderDepth() ? RenderTargetType::COLOR_AND_DEPTH : RenderTargetType::COLOR,
                       converter::convertVkFormat(format), extent.width, extent.height);
    m_device = &device;
    m_swapchain = &swapchain;
    m_extent = extent;

    vkGetSwapchainImagesKHR(m_device->getLogical(), m_swapchain->get(), &m_imageCount, nullptr);
    std::vector<VkImage> images(m_imageCount);
    vkGetSwapchainImagesKHR(m_device->getLogical(), m_swapchain->get(), &m_imageCount, images.data());

    m_texture.init(device, images, format, extent, *this);

    if (swapchain.renderDepth())
    {
        m_depthTexture.init(device, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, extent.width, extent.height,
                            m_imageCount, *this);
    }

    for (auto& renderPass : m_renderPasses)
    {
        renderPass->createFramebuffers();
    }
}

void VulkanRenderTarget::init(Device& device, RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height)
{
    RenderTarget::init(type, format, width, height);
    m_device = &device;
    m_swapchain = nullptr;
    m_extent = {.width = width, .height = height};
    m_imageCount = GraphicsManager::MAX_FRAMES_IN_FLIGHT;

    if (type == RenderTargetType::COLOR || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_texture.init(device, TextureType::COLOR, format, width, height, m_imageCount, *this);
    }
    if (type == RenderTargetType::DEPTH || type == RenderTargetType::COLOR_AND_DEPTH)
    {
        m_depthTexture.init(device, TextureType::DEPTH, GraphicsDataFormat::UNDEFINED, width, height, m_imageCount,
                            *this);
    }

    for (auto& renderPass : m_renderPasses)
    {
        renderPass->createFramebuffers();
    }
}

void VulkanRenderTarget::cleanup()
{
    partialCleanup();
    m_renderPasses.clear();
}

void VulkanRenderTarget::partialCleanup()
{
    for (auto& renderPass : m_renderPasses)
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

void VulkanRenderTarget::addRenderPass(VulkanRenderPass* renderPass) { m_renderPasses.push_back(renderPass); }

Ref<Texture> VulkanRenderTarget::getColorTexture()
{
    if (usesColor())
    {
        return Ref<Texture>(&m_texture);
    }
    return Ref<Texture>(nullptr);
}

Ref<Texture> VulkanRenderTarget::getDepthTexture()
{
    if (usesDepth())
    {
        return Ref<Texture>(&m_depthTexture);
    }
    return Ref<Texture>(nullptr);
}

void VulkanRenderTarget::setAvailability(bool available) { RenderTarget::setAvailability(available); }

} // namespace huedra
