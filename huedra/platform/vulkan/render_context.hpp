#pragma once

#include "graphics/render_context.hpp"
#include "platform/vulkan/buffer.hpp"
#include "platform/vulkan/render_pass.hpp"
#include "platform/vulkan/resource_set.hpp"

namespace huedra {

class VulkanRenderContext : public RenderContext
{
public:
    VulkanRenderContext() = default;
    ~VulkanRenderContext() = default;

    void init(VkCommandBuffer commandBuffer, VulkanRenderPass* renderPass);

    void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) override;
    void bindIndexBuffer(Ref<Buffer> buffer) override;
    void bindResourceSets(std::vector<Ref<ResourceSet>> resourceSets) override;
    void bindResourceSet(Ref<ResourceSet> resourceSet) override;

    void pushConstants(ShaderStageFlags shaderStage, u32 size, void* data) override;

    void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) override;
    void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) override;

private:
    VkCommandBuffer m_commandBuffer;
    VulkanRenderPass* p_renderPass;

    bool m_boundVertexBuffer{false};
    bool m_boundIndexBuffer{false};
};

} // namespace huedra