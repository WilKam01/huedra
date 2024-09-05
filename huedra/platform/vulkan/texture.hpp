#pragma once

#include "graphics/pipeline_data.hpp"
#include "graphics/texture.hpp"
#include "platform/vulkan/command_pool.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

class VulkanTexture : public Texture
{
public:
    VulkanTexture() = default;
    ~VulkanTexture() = default;

    void init(Device& device, CommandPool& commandPool, GraphicsDataFormat format, u32 width, u32 height, u32 texelSize,
              void* data);
    void init(Device& device, CommandPool& commandPool, TextureType type, GraphicsDataFormat format, u32 width,
              u32 height, u32 imageCount);
    void init(Device& device, CommandPool& commandPool, std::vector<VkImage> images, VkFormat format,
              VkExtent2D extent);
    void cleanup() override;

    VkImage get(size_t index = 0) { return m_images[index]; }
    VkImageView getView(size_t index = 0) { return m_imageViews[index]; }
    VkFormat getFormat() { return m_format; }
    VkSampler getSampler() { return m_sampler; }

private:
    VkFormat findFormat(TextureType type, GraphicsDataFormat format);

    void createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void createImageViews(VkImageAspectFlags aspectFlags);

    Device* p_device;
    CommandPool* p_commandPool;
    bool m_externallyCreated{false};
    bool m_multipleImages{false};
    bool m_createdSampler{false};

    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<VkImageView> m_imageViews;
    VkFormat m_format;
    VkSampler m_sampler;
};

} // namespace huedra
