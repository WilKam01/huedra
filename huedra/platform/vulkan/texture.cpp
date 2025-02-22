#include "texture.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/render_target.hpp"
#include "platform/vulkan/swapchain.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanTexture::init(Device& device, const TextureData& textureData, VkFormat format, VkImage image,
                         VkDeviceMemory memory)
{
    Texture::init(textureData.width, textureData.height, textureData.format, TextureType::COLOR);

    m_device = &device;
    m_externallyCreated = false;
    m_format = format;

    m_images.push_back(image);
    m_memories.push_back(memory);
    m_imageViews.resize(1);
    m_imageLayouts.resize(1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_layoutStages.resize(1, VK_PIPELINE_STAGE_TRANSFER_BIT);

    createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::init(Device& device, TextureType type, GraphicsDataFormat format, u32 width, u32 height,
                         u32 imageCount, VulkanRenderTarget& renderTarget)
{
    Texture::init(width, height, format, type);

    m_device = &device;
    m_renderTarget = &renderTarget;
    m_externallyCreated = false;
    m_format = findFormat(type, format);

    m_images.resize(imageCount);
    m_imageViews.resize(imageCount);
    m_memories.resize(imageCount);
    m_imageLayouts.resize(imageCount, VK_IMAGE_LAYOUT_UNDEFINED);
    m_layoutStages.resize(imageCount, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    if (type == TextureType::COLOR)
    {
        createImages(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);
    }
    else
    {
        createImages(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        createImageViews(VK_IMAGE_ASPECT_DEPTH_BIT);
    }
}

void VulkanTexture::init(Device& device, std::vector<VkImage> images, VkFormat format, VkExtent2D extent,
                         VulkanRenderTarget& renderTarget)
{
    Texture::init(extent.width, extent.height, converter::convertVkFormat(format), TextureType::COLOR);

    m_device = &device;
    m_renderTarget = &renderTarget;
    m_externallyCreated = true;
    m_format = format;

    m_images = std::move(images);
    m_imageViews.resize(m_images.size());
    m_memories.resize(m_images.size());
    m_imageLayouts.resize(m_images.size(), VK_IMAGE_LAYOUT_UNDEFINED);
    m_layoutStages.resize(m_images.size(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::cleanup()
{
    for (u64 i = 0; i < m_images.size(); ++i)
    {
        vkDestroyImageView(m_device->getLogical(), m_imageViews[i], nullptr);
        if (!m_externallyCreated)
        {
            vkDestroyImage(m_device->getLogical(), m_images[i], nullptr);
            vkFreeMemory(m_device->getLogical(), m_memories[i], nullptr);
        }
    }
    m_imageViews.clear();
    m_images.clear();
    m_memories.clear();
    m_imageLayouts.clear();
    m_layoutStages.clear();
    m_renderTarget = nullptr;
}

u32 VulkanTexture::getIndex() const
{
    if (m_renderTarget != nullptr)
    {
        return (m_renderTarget->getSwapchain() != nullptr ? m_renderTarget->getSwapchain()->getImageIndex()
                                                          : global::graphicsManager.getCurrentFrame());
    }
    return 0;
}

VkFormat VulkanTexture::findFormat(TextureType type, GraphicsDataFormat format)
{
    if (type == TextureType::DEPTH)
    {
        return m_device->findDepthFormat();
    }

    VkFormat vkFormat = converter::convertDataFormat(format);
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_device->getPhysical(), vkFormat, &formatProperties);

    if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0u)
    {
        return vkFormat;
    }

    log(LogLevel::ERR, "Failed to find supported format!");
    return VK_FORMAT_UNDEFINED;
}

void VulkanTexture::createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    for (u64 i = 0; i < m_images.size(); ++i)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = getWidth();
        imageInfo.extent.height = getHeight();
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_device->getLogical(), &imageInfo, nullptr, &m_images[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device->getLogical(), m_images[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_device->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_device->getLogical(), &allocInfo, nullptr, &m_memories[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to allocate image memory!");
        }

        vkBindImageMemory(m_device->getLogical(), m_images[i], m_memories[i], 0);
    }
}

void VulkanTexture::createImageViews(VkImageAspectFlags aspectFlags)
{
    for (u64 i = 0; i < m_images.size(); ++i)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device->getLogical(), &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create image view!");
        }
    }
}

} // namespace huedra