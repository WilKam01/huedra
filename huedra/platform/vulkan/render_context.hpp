#pragma once

#include "graphics/render_context.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/descriptor_handler.hpp"
#include "platform/vulkan/render_pass.hpp"

namespace huedra {

class VulkanContext;

class VulkanRenderContext : public RenderContext
{
public:
    VulkanRenderContext() = default;
    ~VulkanRenderContext() = default;

    void init(VkCommandBuffer commandBuffer, VulkanContext* context, VulkanRenderPass* renderPass,
              DescriptorHandler& descriptorHandler);

    void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) override;
    void bindIndexBuffer(Ref<Buffer> buffer) override;
    void bindBuffer(Ref<Buffer> buffer, u32 set, u32 binding) override;
    void bindTexture(Ref<Texture> texture, u32 set, u32 binding) override;

    void pushConstants(ShaderStageFlags shaderStage, u32 size, void* data) override;

    void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) override;
    void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) override;

private:
    VkCommandBuffer m_commandBuffer;
    VulkanContext* p_context;
    VulkanRenderPass* p_renderPass;
    DescriptorHandler* p_descriptorHandler;

    bool m_boundVertexBuffer{false};
    bool m_boundIndexBuffer{false};
};

} // namespace huedra