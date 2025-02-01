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

    void init(Device& device, const TextureData& textureData, VkFormat format, VkImage image,
              VkDeviceMemory memory); // Static texture
    void init(Device& device, TextureType type, GraphicsDataFormat format, u32 width, u32 height, u32 imageCount,
              VulkanRenderTarget& renderTarget); // Render target texture
    void init(Device& device, std::vector<VkImage> images, VkFormat format, VkExtent2D extent,
              VulkanRenderTarget& renderTarget); // Render target texture with swap chain
    void cleanup();

    VkImage get() { return m_images[getIndex()]; }
    VkImageView getView() { return m_imageViews[getIndex()]; }
    VkImageView getView(u32 index) { return m_imageViews[index]; }
    VkImageLayout getLayout() { return m_imageLayouts[getIndex()]; }
    VkPipelineStageFlags getLayoutStage() { return m_layoutStages[getIndex()]; }
    VulkanRenderTarget* getRenderTarget() { return p_renderTarget; }
    VkFormat getFormat() { return m_format; }
    VkSampler getSampler() { return m_sampler; }

    void setLayout(VkImageLayout layout) { m_imageLayouts[getIndex()] = layout; }
    void setLayoutStage(VkPipelineStageFlags stage) { m_layoutStages[getIndex()] = stage; }

private:
    u32 getIndex();
    VkFormat findFormat(TextureType type, GraphicsDataFormat format);
    void createSampler(); // TODO: Separate from here to context, also add user control and multiple samplers

    void createImages(VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void createImageViews(VkImageAspectFlags aspectFlags);

    Device* p_device;
    VulkanRenderTarget* p_renderTarget{nullptr};
    bool m_externallyCreated{false};
    bool m_createdSampler{false};

    VkFormat m_format;
    std::vector<VkImage> m_images;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkImageLayout> m_imageLayouts;
    std::vector<VkPipelineStageFlags> m_layoutStages;
    VkSampler m_sampler;
};

} // namespace huedra