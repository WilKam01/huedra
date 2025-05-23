#pragma once

#include "graphics/pipeline_data.hpp"
#include "graphics/texture.hpp"
#include "platform/vulkan/command_pool.hpp"
#include "platform/vulkan/device.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

class VulkanRenderTarget;

class VulkanTexture : public Texture
{
public:
    VulkanTexture() = default;
    ~VulkanTexture() = default;

    VulkanTexture(const VulkanTexture& rhs) = default;
    VulkanTexture& operator=(const VulkanTexture& rhs) = default;
    VulkanTexture(VulkanTexture&& rhs) = default;
    VulkanTexture& operator=(VulkanTexture&& rhs) = default;

    void init(Device& device, const TextureData& textureData, VkFormat format, VkImage image,
              VkDeviceMemory memory); // Static texture
    void init(Device& device, TextureType type, GraphicsDataFormat format, u32 width, u32 height, u32 imageCount,
              VulkanRenderTarget& renderTarget); // Render target texture
    void init(Device& device, std::vector<VkImage> images, VkFormat format, VkExtent2D extent,
              VulkanRenderTarget& renderTarget); // Render target texture with swap chain
    void cleanup();

    VkImage get() const { return m_images[getIndex()]; }
    VkImageView getView() const { return m_imageViews[getIndex()]; }
    VkImageView getView(u32 index) const { return m_imageViews[index]; }
    VkImageLayout getLayout() const { return m_imageLayouts[getIndex()]; }
    VkPipelineStageFlags getLayoutStage() const { return m_layoutStages[getIndex()]; }
    VulkanRenderTarget* getRenderTarget() const { return m_renderTarget; }
    VkFormat getFormat() const { return m_format; }

    void setLayout(VkImageLayout layout) { m_imageLayouts[getIndex()] = layout; }
    void setLayoutStage(VkPipelineStageFlags stage) { m_layoutStages[getIndex()] = stage; }

private:
    u32 getIndex() const;
    VkFormat findFormat(TextureType type, GraphicsDataFormat format);

    void createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void createImageViews(VkImageAspectFlags aspectFlags);

    Device* m_device{nullptr};
    VulkanRenderTarget* m_renderTarget{nullptr};
    bool m_externallyCreated{false};

    VkFormat m_format{VK_FORMAT_UNDEFINED};
    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkImageLayout> m_imageLayouts;
    std::vector<VkPipelineStageFlags> m_layoutStages;
};

} // namespace huedra