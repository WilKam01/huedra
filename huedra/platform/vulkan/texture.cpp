#include "texture.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/pipeline.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanTexture::init(Device& device, CommandPool& commandPool, TextureData textureData)
{
    Texture::init(textureData.width, textureData.height, textureData.format, TextureType::COLOR);

    p_device = &device;
    p_commandPool = &commandPool;
    m_externallyCreated = false;
    m_multipleImages = false;
    m_createdSampler = true;

    m_format = findFormat(TextureType::COLOR, textureData.format);

    VkDeviceSize size = textureData.width * textureData.height * textureData.texelSize;

    m_images.resize(1);
    m_imageViews.resize(1);
    m_memories.resize(1);

    VulkanBuffer stagingBuffer;
    stagingBuffer.init(device, BufferType::STATIC, HU_BUFFER_USAGE_UNDEFINED, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       textureData.texels.data());

    createImages(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    commandPool.transistionImageLayout(
        m_images[0], m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_NONE,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkCommandBuffer commandBuffer = commandPool.beginSingleTimeCommand();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {textureData.width, textureData.height, 1};

    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer.get(), m_images[0], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);

    commandPool.endSingleTimeCommand(commandBuffer);

    commandPool.transistionImageLayout(m_images[0], m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
                                       VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    stagingBuffer.cleanup();

    createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(p_device->getPhysical(), &properties);

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(p_device->getLogical(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create sampler!");
    }
}

void VulkanTexture::init(Device& device, CommandPool& commandPool, TextureType type, GraphicsDataFormat format,
                         u32 width, u32 height, u32 imageCount)
{
    Texture::init(width, height, format, type);

    p_device = &device;
    p_commandPool = &commandPool;
    m_externallyCreated = false;
    m_multipleImages = true;
    m_createdSampler = false;

    m_format = findFormat(type, format);

    m_images.resize(imageCount);
    m_imageViews.resize(imageCount);
    m_memories.resize(imageCount);

    if (type == TextureType::DEPTH)
    {
        createImages(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        createImageViews(VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    else
    {
        createImages(VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void VulkanTexture::init(Device& device, CommandPool& commandPool, std::vector<VkImage> images, VkFormat format,
                         VkExtent2D extent)
{
    Texture::init(extent.width, extent.height, converter::convertVkFormat(format), TextureType::COLOR);

    p_device = &device;
    p_commandPool = &commandPool;
    m_externallyCreated = true;
    m_multipleImages = true;
    m_createdSampler = false;
    m_format = format;

    m_images = images;
    m_imageViews.resize(m_images.size());
    m_memories.resize(m_images.size());
    createImageViews(VK_IMAGE_ASPECT_COLOR_BIT);
}

void VulkanTexture::cleanup()
{
    if (m_createdSampler)
    {
        vkDestroySampler(p_device->getLogical(), m_sampler, nullptr);
    }

    for (size_t i = 0; i < m_images.size(); ++i)
    {
        vkDestroyImageView(p_device->getLogical(), m_imageViews[i], nullptr);
        if (!m_externallyCreated)
        {
            vkDestroyImage(p_device->getLogical(), m_images[i], nullptr);
            vkFreeMemory(p_device->getLogical(), m_memories[i], nullptr);
        }
    }
    m_imageViews.clear();
    m_images.clear();
    m_memories.clear();
}

VkFormat VulkanTexture::findFormat(TextureType type, GraphicsDataFormat format)
{
    if (type == TextureType::DEPTH)
    {
        return p_device->findDepthFormat();
    }
    else
    {
        VkFormat vkFormat = converter::convertDataFormat(format);
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(p_device->getPhysical(), vkFormat, &formatProperties);

        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {
            return vkFormat;
        }
    }

    log(LogLevel::ERR, "Failed to find supported format!");
    return VK_FORMAT_UNDEFINED;
}

void VulkanTexture::createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
{
    for (size_t i = 0; i < m_images.size(); ++i)
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

        if (vkCreateImage(p_device->getLogical(), &imageInfo, nullptr, &m_images[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(p_device->getLogical(), m_images[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = p_device->findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(p_device->getLogical(), &allocInfo, nullptr, &m_memories[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to allocate image memory!");
        }

        vkBindImageMemory(p_device->getLogical(), m_images[i], m_memories[i], 0);
    }
}

void VulkanTexture::createImageViews(VkImageAspectFlags aspectFlags)
{
    for (size_t i = 0; i < m_images.size(); ++i)
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

        if (vkCreateImageView(p_device->getLogical(), &viewInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create image view!");
        }
    }
}

} // namespace huedra