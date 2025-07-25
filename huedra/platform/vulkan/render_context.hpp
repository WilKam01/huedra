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
    ~VulkanRenderContext() override = default;

    VulkanRenderContext(const VulkanRenderContext& rhs) = delete;
    VulkanRenderContext& operator=(const VulkanRenderContext& rhs) = delete;
    VulkanRenderContext(VulkanRenderContext&& rhs) = delete;
    VulkanRenderContext& operator=(VulkanRenderContext&& rhs) = delete;

    void init(VkCommandBuffer commandBuffer, VulkanContext* context, VulkanRenderPass* renderPass,
              DescriptorHandler& descriptorHandler);

    void bindVertexBuffers(std::vector<Ref<Buffer>> buffers) override;
    void bindIndexBuffer(Ref<Buffer> buffer) override;

    void bindBuffer(Ref<Buffer> buffer, std::string_view name) override;
    void bindTexture(Ref<Texture> texture, std::string_view name) override;
    void bindSampler(const SamplerSettings& sampler, std::string_view name) override;
    void setParameter(void* data, u32 size, std::string_view name) override;

    void draw(u32 vertexCount, u32 instanceCount, u32 vertexOffset, u32 instanceOffset) override;
    void drawIndexed(u32 indexCount, u32 instanceCount, u32 indexOffset, u32 instanceOffset) override;
    void dispatchGroups(u32 groupX, u32 groupY, u32 groupZ) override;
    void dispatch(u32 x, u32 y, u32 z) override;

    uvec3 getComputeThreadsPerGroup() override
    {
        return m_renderPass->getPipeline().getShaderModule().getComputeThreadsPerGroup();
    }

private:
    VkCommandBuffer m_commandBuffer{nullptr};
    VulkanContext* m_context{nullptr};
    VulkanRenderPass* m_renderPass{nullptr};
    DescriptorHandler* m_descriptorHandler{nullptr};

    bool m_boundIndexBuffer{false};
};

} // namespace huedra