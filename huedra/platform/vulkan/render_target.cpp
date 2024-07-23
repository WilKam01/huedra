#include "render_target.hpp"
#include "core/log.hpp"
#include "graphics/graphics_manager.hpp"
#include "platform/vulkan/swapchain.hpp"

namespace huedra {

void VulkanRenderTarget::init(Device& device, VkFormat format, VkExtent2D extent, VkRenderPass renderPass,
                              VulkanSwapchain* swapchain)
{
    RenderTarget::init(extent.width, extent.height);
    p_device = &device;
    p_swapchain = swapchain;
    m_format = format;
    m_extent = extent;

    if (p_swapchain)
    {
        u32 imageCount;
        vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &imageCount, nullptr);
        m_images.resize(imageCount);
        vkGetSwapchainImagesKHR(p_device->getLogical(), p_swapchain->get(), &imageCount, m_images.data());
    }
    else
    {
        m_images.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        m_imageMemories.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_format;
        imageInfo.extent.width = m_extent.width;
        imageInfo.extent.height = m_extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        for (size_t i = 0; i < m_images.size(); ++i)
        {
            if (vkCreateImage(p_device->getLogical(), &imageInfo, nullptr, &m_images[i]) != VK_SUCCESS)
            {
                log(LogLevel::ERR, "Failed to create render target image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(p_device->getLogical(), m_images[i], &memRequirements);

            VkMemoryAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex =
                p_device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            if (vkAllocateMemory(device.getLogical(), &allocInfo, nullptr, &m_imageMemories[i]) != VK_SUCCESS)
            {
                log(LogLevel::ERR, "Failed to allocate render target image memory!");
            }

            vkBindImageMemory(p_device->getLogical(), m_images[i], m_imageMemories[i], 0);
        }
    }

    createImageViews();
    createFramebuffers(renderPass);
}

void VulkanRenderTarget::cleanup()
{
    for (size_t i = 0; i < m_images.size(); ++i)
    {
        vkDestroyFramebuffer(p_device->getLogical(), m_framebuffers[i], nullptr);
        vkDestroyImageView(p_device->getLogical(), m_imageViews[i], nullptr);
        if (!p_swapchain)
        {
            vkFreeMemory(p_device->getLogical(), m_imageMemories[i], nullptr);
            vkDestroyImage(p_device->getLogical(), m_images[i], nullptr);
        }
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
        m_currentImageIndex = (m_currentImageIndex + 1) % GraphicsManager::MAX_FRAMES_IN_FLIGHT;
        setAvailability(true);
    }
}

void VulkanRenderTarget::createImageViews()
{
    size_t size = m_images.size();
    m_imageViews.resize(size);

    for (size_t i = 0; i < size; i++)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(p_device->getLogical(), &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create render target image view!");
        }
    }
}

void VulkanRenderTarget::createFramebuffers(VkRenderPass renderPass)
{
    size_t size = m_imageViews.size();
    m_framebuffers.resize(size);

    for (size_t i = 0; i < size; i++)
    {
        std::array<VkImageView, 1> attachments = {m_imageViews[i]};

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
