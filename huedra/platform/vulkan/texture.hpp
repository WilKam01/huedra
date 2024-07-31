#pragma once

#include "graphics/pipeline_data.hpp"
#include "platform/vulkan/command_pool.hpp"
#include "platform/vulkan/device.hpp"

namespace huedra {

enum class TextureType
{
    COLOR,
    DEPTH,
};

class VulkanTexture
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
    void cleanup();

    VkImage get(size_t index = 0) { return m_images[index]; }
    VkImageView getView(size_t index = 0) { return m_imageViews[index]; }

private:
    VkFormat findFormat(TextureType type, GraphicsDataFormat format);

    void createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void createImageViews(VkImageAspectFlags aspectFlags);

    Device* p_device;
    CommandPool* p_commandPool;
    bool m_externallyCreated{false};
    bool m_multipleImages{true};

    u32 m_width;
    u32 m_height;
    TextureType m_type;
    GraphicsDataFormat m_dataFormat;

    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<VkImageView> m_imageViews;
    VkFormat m_format;
};

} // namespace huedra
