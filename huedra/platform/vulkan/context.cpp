#include "context.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "core/string/utils.hpp"
#include "platform/vulkan/config.hpp"
#include "platform/vulkan/os_manager.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanContext::init()
{
    m_instance.init();

    Window* tempWindow = global::windowManager.createWindow("temp", {});
    VkSurfaceKHR surface = createSurface(m_instance, tempWindow);

    m_device.init(m_instance, surface);

    m_graphicsCommandPool.init(m_device, VK_PIPELINE_BIND_POINT_GRAPHICS);
    m_computeCommandPool.init(m_device, VK_PIPELINE_BIND_POINT_COMPUTE);

    m_graphicsCommandBuffer.init(m_device, m_graphicsCommandPool, GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeCommandBuffer.init(m_device, m_computeCommandPool, GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    m_graphicsFrameInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeFrameInFlightFences.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[0].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_graphicsSyncSemaphores[1].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeSyncSemaphores[0].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    m_computeSyncSemaphores[1].resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_graphicsFrameInFlightFences[i]) !=
                VK_SUCCESS ||
            vkCreateFence(m_device.getLogical(), &fenceInfo, nullptr, &m_computeFrameInFlightFences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[0][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_graphicsSyncSemaphores[1][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_computeSyncSemaphores[0][i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.getLogical(), &semaphoreInfo, nullptr, &m_computeSyncSemaphores[1][i]) !=
                VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create render fences and semaphores!");
        }
    }

    vkDestroySurfaceKHR(m_instance.get(), surface, nullptr);
    tempWindow->cleanup();
    delete tempWindow;
}

void VulkanContext::cleanup()
{
    for (auto& buffer : m_buffers)
    {
        buffer.cleanup();
    }
    m_buffers.clear();

    for (auto& texture : m_textures)
    {
        texture.cleanup();
    }
    m_textures.clear();

    for (auto& renderTarget : m_renderTargets)
    {
        renderTarget.cleanup();
    }
    m_renderTargets.clear();

    for (auto& batch : m_passBatches)
    {
        for (auto& renderPass : batch.passes)
        {
            renderPass.pass->cleanup();
            delete renderPass.pass;
            for (auto& handler : renderPass.descriptorHandlers)
            {
                handler.cleanup();
            }
            vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
        }
    }
    m_passBatches.clear();
    m_activeSwapchains.clear();
    m_usingGraphicsQueue = false;
    m_usingComputeQueue = false;

    for (auto& info : m_samplers)
    {
        vkDestroySampler(m_device.getLogical(), info.sampler, nullptr);
    }
    m_samplers.clear();

    for (auto& swapchain : m_swapchains)
    {
        swapchain->cleanup();
        delete swapchain;
    }
    m_swapchains.clear();

    for (u32 i = 0; i < GraphicsManager::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(m_device.getLogical(), m_computeSyncSemaphores[0][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_computeSyncSemaphores[1][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[0][i], nullptr);
        vkDestroySemaphore(m_device.getLogical(), m_graphicsSyncSemaphores[1][i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_computeFrameInFlightFences[i], nullptr);
        vkDestroyFence(m_device.getLogical(), m_graphicsFrameInFlightFences[i], nullptr);
    }

    m_computeCommandBuffer.cleanup();
    m_graphicsCommandBuffer.cleanup();

    m_computeCommandPool.cleanup();
    m_graphicsCommandPool.cleanup();

    m_device.cleanup();
    m_instance.cleanup();
}

void VulkanContext::createSwapchain(Window* window, bool renderDepth)
{
    auto* swapchain = new VulkanSwapchain();
    VkSurfaceKHR surface = createSurface(m_instance, window);
    swapchain->init(window, m_device, surface, renderDepth);

    m_swapchains.push_back(swapchain);
    m_surfaces.push_back(surface);
}

void VulkanContext::removeSwapchain(u64 index)
{
    VulkanSwapchain* swapchain = m_swapchains[index];
    m_swapchains.erase(m_swapchains.begin() + static_cast<i64>(index));

    swapchain->cleanup();
    delete swapchain;

    vkDestroySurfaceKHR(m_instance.get(), m_surfaces[index], nullptr);
    m_surfaces.erase(m_surfaces.begin() + static_cast<i64>(index));
}

Buffer* VulkanContext::createBuffer(BufferType type, BufferUsageFlags usage, u64 size, void* data)
{
    VulkanBuffer& buffer = m_buffers.emplace_back();
    VkBufferUsageFlagBits bufferUsage = converter::convertBufferUsage(usage);

    if (type == BufferType::STATIC && data != nullptr)
    {
        m_stagingBuffer.init(m_device, BufferType::STATIC, size, HU_BUFFER_USAGE_UNDEFINED,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);

        buffer.init(m_device, type, size, usage, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer commandBuffer = m_graphicsCommandPool.beginSingleTimeCommand();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = static_cast<VkDeviceSize>(size);
        vkCmdCopyBuffer(commandBuffer, m_stagingBuffer.get(), buffer.get(), 1, &copyRegion);
        m_graphicsCommandPool.endSingleTimeCommand(commandBuffer);

        m_stagingBuffer.cleanup();
    }
    else
    {
        buffer.init(m_device, type, size, usage, bufferUsage,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data);
    }

    return &buffer;
}

Texture* VulkanContext::createTexture(const TextureData& textureData)
{
    VulkanTexture& texture = m_textures.emplace_back();

    auto size = static_cast<u64>(textureData.width) * static_cast<u64>(textureData.height) *
                static_cast<u64>(textureData.texelSize);
    VkFormat format = converter::convertDataFormat(textureData.format);
    VkImage image = nullptr;
    VkDeviceMemory memory = nullptr;

    m_stagingBuffer.init(
        m_device, BufferType::STATIC, size, HU_BUFFER_USAGE_UNDEFINED, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, textureData.texels.data());

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = textureData.width;
    imageInfo.extent.height = textureData.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device.getLogical(), &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device.getLogical(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        m_device.findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device.getLogical(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to allocate image memory!");
    }

    vkBindImageMemory(m_device.getLogical(), image, memory, 0);

    VkCommandBuffer commandBuffer = m_graphicsCommandPool.beginSingleTimeCommand();

    transitionImageLayout(commandBuffer, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_ACCESS_NONE, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {.x = 0, .y = 0, .z = 0};
    region.imageExtent = {.width = textureData.width, .height = textureData.height, .depth = 1};

    vkCmdCopyBufferToImage(commandBuffer, m_stagingBuffer.get(), image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);

    m_graphicsCommandPool.endSingleTimeCommand(commandBuffer);

    m_stagingBuffer.cleanup();

    texture.init(m_device, textureData, format, image, memory);
    return &texture;
}

RenderTarget* VulkanContext::createRenderTarget(RenderTargetType type, GraphicsDataFormat format, u32 width, u32 height)
{
    VulkanRenderTarget& renderTarget = m_renderTargets.emplace_back();
    renderTarget.init(m_device, type, format, width, height);
    return &renderTarget;
}

void VulkanContext::removeBuffer(Buffer* buffer)
{
    auto it = std::ranges::find_if(m_buffers, [&](VulkanBuffer& vkBuffer) { return &vkBuffer == buffer; });
    if (it != m_buffers.end())
    {
        it->cleanup();
        m_buffers.erase(it);
    }
}

void VulkanContext::removeTexture(Texture* texture)
{
    auto it = std::ranges::find_if(m_textures, [&](VulkanTexture& vkTexture) { return &vkTexture == texture; });
    if (it != m_textures.end())
    {
        it->cleanup();
        m_textures.erase(it);
    }
}

void VulkanContext::removeRenderTarget(RenderTarget* renderTarget)
{
    auto it = std::ranges::find_if(m_renderTargets,
                                   [&](VulkanRenderTarget& vkRenderTarget) { return &vkRenderTarget == renderTarget; });
    if (it != m_renderTargets.end())
    {
        it->cleanup();
        m_renderTargets.erase(it);
    }
}

void VulkanContext::prepareSwapchains()
{
    for (auto& swapchain : m_swapchains)
    {
        swapchain->aquireNextImage();
    }
}

void VulkanContext::setRenderGraph(RenderGraphBuilder& builder)
{
    if (m_curGraph.getHash() == builder.getHash())
    {
        return;
    }

    m_curGraph = builder;
    m_device.waitIdle();

    log(LogLevel::INFO, "New render graph with hash: 0x{:x}", m_curGraph.getHash());

    // Destroy all previous batches
    for (auto& batch : m_passBatches)
    {
        for (auto& renderPass : batch.passes)
        {
            renderPass.pass->cleanup();
            delete renderPass.pass;
            for (auto& handler : renderPass.descriptorHandlers)
            {
                handler.cleanup();
            }
            vkDestroyDescriptorPool(m_device.getLogical(), renderPass.descriptorPool, nullptr);
        }
    }
    m_passBatches.clear();
    m_activeSwapchains.clear();
    m_usingGraphicsQueue = false;
    m_usingComputeQueue = false;

    for (auto& info : m_samplers)
    {
        vkDestroySampler(m_device.getLogical(), info.sampler, nullptr);
    }
    m_samplers.clear();

    struct VersionData
    {
        u32 version{0};
        VkImageLayout curLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        TextureType textureType{TextureType::COLOR};
        VulkanRenderPass* firstRenderPass{nullptr};
        VulkanRenderPass* curRenderPass{nullptr};
        u32 curRenderTargetIndex{0};
    };
    std::map<void*, VersionData> resourceVersions; // Keeping track on resource iterations

    m_passBatches.emplace_back(); // At least one batch
    for (auto& [key, info] : builder.getRenderPasses())
    {
        PassInfo passInfo;
        passInfo.pass = new VulkanRenderPass();
        passInfo.pass->init(m_device, info);

        std::vector<VulkanSwapchain*> swapchains;
        u32 latestVersion = 0; // Checking the latest iteration of an input resource
        for (auto& input : info.getInputs())
        {
            void* ptr = input.type == RenderPassReference::Type::BUFFER ? static_cast<void*>(input.buffer)
                                                                        : static_cast<void*>(input.texture);
            if (!resourceVersions.contains(ptr))
            {
                resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
            }
            latestVersion = std::max(latestVersion, resourceVersions[ptr].version);
            resourceVersions[ptr].curLayout = resourceVersions[ptr].textureType == TextureType::COLOR
                                                  ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                  : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            if (resourceVersions[ptr].curRenderPass != nullptr)
            {
                if (resourceVersions[ptr].textureType == TextureType::COLOR)
                {
                    resourceVersions[ptr].curRenderPass->setFinalColorLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
                else
                {
                    resourceVersions[ptr].curRenderPass->setFinalDepthLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
            }
            else if (input.type == RenderPassReference::Type::TEXTURE)
            {
                ResourceTransition& transition = passInfo.transitions.emplace_back();
                transition.texture = static_cast<VulkanTexture*>(input.texture);
                transition.newLayout = resourceVersions[ptr].curLayout;
                transition.newStage = converter::convertPipelineStage(info.getPipeline().getType(), input.shaderStage);
            }
            resourceVersions[ptr].curRenderPass = nullptr;

            if (input.type == RenderPassReference::Type::TEXTURE)
            {
                auto* texture = static_cast<VulkanTexture*>(input.texture);
                if (texture->getRenderTarget() != nullptr && texture->getRenderTarget()->getSwapchain() != nullptr)
                {
                    swapchains.push_back(texture->getRenderTarget()->getSwapchain());
                    m_activeSwapchains.insert(texture->getRenderTarget()->getSwapchain());
                }
            }
        }

        for (auto& output : info.getOutputs())
        {
            void* ptr = output.type == RenderPassReference::Type::BUFFER ? static_cast<void*>(output.buffer)
                                                                         : static_cast<void*>(output.texture);
            if (!resourceVersions.contains(ptr))
            {
                resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
            }
            resourceVersions[ptr].version = latestVersion + 1;
            resourceVersions[ptr].curLayout = VK_IMAGE_LAYOUT_GENERAL;
            if (resourceVersions[ptr].curRenderPass != nullptr)
            {
                if (resourceVersions[ptr].textureType == TextureType::COLOR)
                {
                    resourceVersions[ptr].curRenderPass->setFinalColorLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
                else
                {
                    resourceVersions[ptr].curRenderPass->setFinalDepthLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
            }
            else if (output.type == RenderPassReference::Type::TEXTURE)
            {
                ResourceTransition& transition = passInfo.transitions.emplace_back();
                transition.texture = static_cast<VulkanTexture*>(output.texture);
                transition.newLayout = resourceVersions[ptr].curLayout;
                transition.newStage = converter::convertPipelineStage(info.getPipeline().getType(), output.shaderStage);
            }
            resourceVersions[ptr].curRenderPass = nullptr;

            if (output.type == RenderPassReference::Type::TEXTURE)
            {
                auto* texture = static_cast<VulkanTexture*>(output.texture);
                if (texture->getRenderTarget() != nullptr && texture->getRenderTarget()->getSwapchain() != nullptr)
                {
                    swapchains.push_back(texture->getRenderTarget()->getSwapchain());
                    m_activeSwapchains.insert(texture->getRenderTarget()->getSwapchain());
                }
            }
        }

        for (u32 i = 0; i < info.getRenderTargets().size(); ++i)
        {
            VulkanRenderTarget* target = static_cast<VulkanRenderTarget*>(info.getRenderTargets()[i].target.get());
            if (target->usesColor())
            {
                void* ptr = &target->getVkColorTexture();
                if (!resourceVersions.contains(ptr))
                {
                    resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
                    resourceVersions[ptr].curLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                else if (resourceVersions[ptr].curLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    resourceVersions[ptr].curLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    resourceVersions[ptr].curRenderPass->setFinalColorLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
                resourceVersions[ptr].version = latestVersion + 1;
                resourceVersions[ptr].curRenderPass = passInfo.pass;
                if (resourceVersions[ptr].firstRenderPass == nullptr)
                {
                    resourceVersions[ptr].firstRenderPass = resourceVersions[ptr].curRenderPass;
                }
                resourceVersions[ptr].curRenderTargetIndex = i;
                resourceVersions[ptr].textureType = TextureType::COLOR;

                if (!resourceVersions[ptr].curRenderPass->getBuilder().getClearRenderTargets())
                {
                    resourceVersions[ptr].curRenderPass->setInitialColorLayout(
                        resourceVersions[ptr].curRenderTargetIndex, resourceVersions[ptr].curLayout);
                }
            }

            if (target->usesDepth())
            {
                void* ptr = &target->getVkDepthTexture();
                if (!resourceVersions.contains(ptr))
                {
                    resourceVersions.insert(std::pair<void*, u32>(ptr, {}));
                }
                else if (resourceVersions[ptr].curLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    resourceVersions[ptr].curLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    resourceVersions[ptr].curRenderPass->setFinalDepthLayout(resourceVersions[ptr].curRenderTargetIndex,
                                                                             resourceVersions[ptr].curLayout);
                }
                resourceVersions[ptr].version = latestVersion + 1;
                resourceVersions[ptr].curRenderPass = passInfo.pass;
                resourceVersions[ptr].curRenderTargetIndex = i;
                resourceVersions[ptr].textureType = TextureType::DEPTH;
                if (resourceVersions[ptr].firstRenderPass == nullptr)
                {
                    resourceVersions[ptr].firstRenderPass = resourceVersions[ptr].curRenderPass;
                }
                if (!resourceVersions[ptr].curRenderPass->getBuilder().getClearRenderTargets())
                {
                    resourceVersions[ptr].curRenderPass->setInitialDepthLayout(
                        resourceVersions[ptr].curRenderTargetIndex, resourceVersions[ptr].curLayout);
                }
            }

            if (target->getSwapchain() != nullptr)
            {
                swapchains.push_back(target->getSwapchain());
                m_activeSwapchains.insert(target->getSwapchain());
            }
        }

        if (m_passBatches.size() <= latestVersion)
        {
            m_passBatches.resize(latestVersion + 1);
        }
        m_passBatches[latestVersion].passes.push_back(passInfo);

        switch (info.getType())
        {
        case RenderPassType::GRAPHICS:
            m_passBatches[latestVersion].useGraphicsQueue = true;
            m_usingGraphicsQueue = true;
            break;
        case RenderPassType::COMPUTE:
            m_passBatches[latestVersion].useComputeQueue = true;
            m_usingComputeQueue = true;
            break;
        }

        for (auto& swapchain : swapchains)
        {
            m_passBatches[latestVersion].swapchains.insert(swapchain);
        }
    }

    for (auto& [ptr, data] : resourceVersions)
    {
        if (data.firstRenderPass != nullptr)
        {
            bool clearTarget = data.firstRenderPass->getBuilder().getClearRenderTargets();
            if (data.textureType == TextureType::COLOR)
            {
                if (!clearTarget)
                {
                    data.firstRenderPass->setInitialColorLayout(data.curRenderTargetIndex, data.curLayout);
                }
                auto* tex = static_cast<VulkanTexture*>(ptr);
                if (tex->getRenderTarget() != nullptr && tex->getRenderTarget()->getSwapchain() != nullptr)
                {
                    if (!clearTarget)
                    {
                        data.firstRenderPass->setInitialColorLayout(data.curRenderTargetIndex,
                                                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                    }
                    if (data.curRenderPass != nullptr)
                    {
                        data.curRenderPass->setFinalColorLayout(data.curRenderTargetIndex,
                                                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
                    }
                }
            }
            else if (!clearTarget)
            {
                data.firstRenderPass->setInitialDepthLayout(data.curRenderTargetIndex, data.curLayout);
            }
        }
    }

    for (auto& batch : m_passBatches)
    {
        for (auto& info : batch.passes)
        {
            info.pass->create();
            createDescriptorHandlers(info.pass->getBuilder(), info);
        }
    }
}

void VulkanContext::render()
{
    m_curGraphicsSemphoreIndex = 0;
    m_curComputeSemphoreIndex = 0;

    std::vector<VkFence> fences{};
    if (m_usingGraphicsQueue)
    {
        fences.push_back(m_graphicsFrameInFlightFences[global::graphicsManager.getCurrentFrame()]);
    }
    if (m_usingComputeQueue)
    {
        fences.push_back(m_computeFrameInFlightFences[global::graphicsManager.getCurrentFrame()]);
    }
    vkWaitForFences(m_device.getLogical(), static_cast<u32>(fences.size()), fences.data(), VK_TRUE, UINT64_MAX);
    vkResetFences(m_device.getLogical(), static_cast<u32>(fences.size()), fences.data());

    for (u32 i = 0; i < m_passBatches.size(); ++i)
    {
        if (m_passBatches[i].useGraphicsQueue)
        {
            m_graphicsCommandBuffer.begin(global::graphicsManager.getCurrentFrame());
        }
        if (m_passBatches[i].useComputeQueue)
        {
            m_computeCommandBuffer.begin(global::graphicsManager.getCurrentFrame());
        }

        for (auto& info : m_passBatches[i].passes)
        {
            VkCommandBuffer commandBuffer{nullptr};
            switch (info.pass->getPipelineType())
            {
            case PipelineType::GRAPHICS:
                commandBuffer = m_graphicsCommandBuffer.get(global::graphicsManager.getCurrentFrame());
                break;
            case PipelineType::COMPUTE:
                commandBuffer = m_computeCommandBuffer.get(global::graphicsManager.getCurrentFrame());
                break;
            }

            VkCommandBuffer transitionCommandBuffer = m_graphicsCommandPool.beginSingleTimeCommand();

            for (auto& transition : info.transitions)
            {
                transitionImageLayout(transitionCommandBuffer, transition.texture->get(),
                                      transition.texture->getFormat(), transition.texture->getLayout(),
                                      transition.newLayout,
                                      vulkan_config::LAYOUT_TO_ACCESS.at(transition.texture->getLayout()),
                                      vulkan_config::LAYOUT_TO_ACCESS.at(transition.newLayout),
                                      transition.texture->getLayoutStage(), transition.newStage);

                transition.texture->setLayout(transition.newLayout);
                transition.texture->setLayoutStage(transition.newStage);
            }

            // TODO: Needs more testing, what stage should be used for read/write textures?
            for (auto& targetInfo : info.pass->getTargetInfo())
            {
                if (targetInfo.renderTarget->usesColor() && targetInfo.initialColorLayout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    VulkanTexture& texture = targetInfo.renderTarget->getVkColorTexture();

                    transitionImageLayout(transitionCommandBuffer, texture.get(), texture.getFormat(),
                                          texture.getLayout(), targetInfo.initialColorLayout,
                                          vulkan_config::LAYOUT_TO_ACCESS.at(texture.getLayout()),
                                          vulkan_config::LAYOUT_TO_ACCESS.at(targetInfo.initialColorLayout),
                                          texture.getLayoutStage(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

                    texture.setLayout(targetInfo.initialColorLayout);
                    texture.setLayoutStage(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                }
                if (targetInfo.renderTarget->usesDepth() && targetInfo.initialDepthLayout != VK_IMAGE_LAYOUT_UNDEFINED)
                {
                    VulkanTexture& texture = targetInfo.renderTarget->getVkDepthTexture();

                    transitionImageLayout(transitionCommandBuffer, texture.get(), texture.getFormat(),
                                          texture.getLayout(), targetInfo.initialDepthLayout,
                                          vulkan_config::LAYOUT_TO_ACCESS.at(texture.getLayout()),
                                          vulkan_config::LAYOUT_TO_ACCESS.at(targetInfo.initialDepthLayout),
                                          texture.getLayoutStage(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

                    texture.setLayout(targetInfo.initialDepthLayout);
                    texture.setLayoutStage(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                }
            }

            m_graphicsCommandPool.endSingleTimeCommand(transitionCommandBuffer);

            info.pass->begin(commandBuffer);

            VulkanRenderContext renderContext;
            renderContext.init(commandBuffer, this, info.pass,
                               info.descriptorHandlers[global::graphicsManager.getCurrentFrame()]);
            info.pass->getCommands()(renderContext);

            info.pass->end(commandBuffer);
        }

        if (m_passBatches[i].useGraphicsQueue)
        {
            submitGraphicsQueue(i);
        }
        if (m_passBatches[i].useComputeQueue)
        {
            submitComputeQueue(i);
        }

        for (const auto& swapchain : m_passBatches[i].swapchains)
        {
            swapchain->setAlreadyWaited();
        }
    }

    for (const auto& swapchain : m_activeSwapchains)
    {
        VulkanTexture& texture = swapchain->getRenderTarget().getVkColorTexture();
        if (texture.getLayout() != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            VkCommandBuffer transitionCommandBuffer = m_graphicsCommandPool.beginSingleTimeCommand();

            transitionImageLayout(transitionCommandBuffer, texture.get(), texture.getFormat(), texture.getLayout(),
                                  VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                  vulkan_config::LAYOUT_TO_ACCESS.at(texture.getLayout()),
                                  vulkan_config::LAYOUT_TO_ACCESS.at(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
                                  texture.getLayoutStage(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

            texture.setLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            texture.setLayoutStage(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

            m_graphicsCommandPool.endSingleTimeCommand(transitionCommandBuffer);
        }
    }

    if (!m_activeSwapchains.empty())
    {
        presentSwapchains();
    }
}

VkSampler VulkanContext::getSampler(const SamplerSettings& settings)
{
    auto it = std::ranges::find_if(m_samplers, [&](const SamplerInfo& info) { return info.settings == settings; });
    if (it != m_samplers.end())
    {
        return it->sampler;
    }

    createSampler(settings);
    return m_samplers.back().sampler;
}

void VulkanContext::createDescriptorHandlers(const RenderPassBuilder& builder, PassInfo& passInfo)
{
    std::vector<std::vector<ResourceBinding>> sets = builder.getPipeline().getResources();
    std::vector<std::vector<VkDescriptorType>> bindingTypes(sets.size());
    std::multiset<VkDescriptorType> poolSizeSet{};
    for (u64 i = 0; i < sets.size(); ++i)
    {
        bindingTypes[i].resize(sets[i].size());
        for (u64 j = 0; j < sets[i].size(); ++j)
        {
            bindingTypes[i][j] = converter::convertResourceType(sets[i][j].resource);
            poolSizeSet.insert(bindingTypes[i][j]);
        }
    }

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (const auto& type : poolSizeSet)
    {
        VkDescriptorPoolSize poolSize;
        poolSize.type = type;
        // TODO: Add ability to decide multiple sets per frame. An estimation or actual?
        poolSize.descriptorCount = static_cast<u32>(poolSizeSet.count(type) * GraphicsManager::MAX_FRAMES_IN_FLIGHT);
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    // TODO: Add ability to decide multiple sets per frame. An estimation or actual?
    poolInfo.maxSets = GraphicsManager::MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(m_device.getLogical(), &poolInfo, nullptr, &passInfo.descriptorPool) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create descriptor pool!");
    }

    passInfo.descriptorHandlers.resize(GraphicsManager::MAX_FRAMES_IN_FLIGHT);
    for (auto& handler : passInfo.descriptorHandlers)
    {
        handler.init(m_device, *passInfo.pass, passInfo.descriptorPool, bindingTypes);
    }
}

void VulkanContext::createSampler(const SamplerSettings& settings)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    VkFilter filter = settings.filter == SamplerFilter::NEAREST ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;

    constexpr auto convertAddressMode = [](SamplerAddressMode addressMode) -> VkSamplerAddressMode {
        switch (addressMode)
        {
        case SamplerAddressMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddressMode::MIRROR_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddressMode::CLAMP_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddressMode::CLAMP_COLOR:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        default:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        };
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };
    samplerInfo.addressModeU = convertAddressMode(settings.adressModeU);
    samplerInfo.addressModeV = convertAddressMode(settings.adressModeV);
    samplerInfo.addressModeW = convertAddressMode(settings.adressModeW);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_device.getPhysical(), &properties);

    VkBorderColor borderColor{VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE};
    switch (settings.color)
    {
    case SamplerColor::WHITE:
        borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case SamplerColor::BLACK:
        borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case SamplerColor::ZERO_ALPHA:
        borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    };

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = borderColor;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode =
        settings.filter == SamplerFilter::NEAREST ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.mipLodBias = 0.0f;

    VkSampler sampler{};
    if (vkCreateSampler(m_device.getLogical(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create sampler!");
    }
    m_samplers.push_back({.settings = settings, .sampler = sampler});
}

void VulkanContext::submitGraphicsQueue(u32 batchIndex)
{
    m_graphicsCommandBuffer.end(global::graphicsManager.getCurrentFrame());

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    for (const auto& swapchain : m_passBatches[batchIndex].swapchains)
    {
        if (!swapchain->alreadyWaited())
        {
            waitSemaphores.push_back(swapchain->getImageAvailableSemaphore());
            waitStages.push_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
    }

    if (batchIndex != 0) // Should wait on queue if it's not the first batch
    {
        if (m_passBatches[batchIndex - 1].useGraphicsQueue)
        {
            waitSemaphores.push_back(
                m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
        if (m_passBatches[batchIndex - 1].useComputeQueue)
        {
            waitSemaphores.push_back(
                m_computeSyncSemaphores[1 - m_curComputeSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_graphicsCommandBuffer.get(global::graphicsManager.getCurrentFrame());

    // Should not signal last batch, EXCEPT for existence of active swapchains
    if (batchIndex != m_passBatches.size() - 1 || !m_activeSwapchains.empty())
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores =
            &m_graphicsSyncSemaphores[m_curGraphicsSemphoreIndex][global::graphicsManager.getCurrentFrame()];
    }

    VkFence fence = m_graphicsFrameInFlightFences[global::graphicsManager.getCurrentFrame()];
    if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit graphics queue!");
    }

    m_curGraphicsSemphoreIndex = 1 - m_curGraphicsSemphoreIndex;
}

void VulkanContext::submitComputeQueue(u32 batchIndex)
{
    m_computeCommandBuffer.end(global::graphicsManager.getCurrentFrame());

    std::vector<VkSemaphore> waitSemaphores;
    std::vector<VkPipelineStageFlags> waitStages;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    for (const auto& swapchain : m_passBatches[batchIndex].swapchains)
    {
        if (!swapchain->alreadyWaited())
        {
            waitSemaphores.push_back(swapchain->getImageAvailableSemaphore());
            waitStages.push_back(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
    }

    if (batchIndex != 0) // Should wait on queue if it's not the first batch
    {
        if (m_passBatches[batchIndex - 1].useGraphicsQueue)
        {
            waitSemaphores.push_back(
                m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
        if (m_passBatches[batchIndex - 1].useComputeQueue)
        {
            waitSemaphores.push_back(
                m_computeSyncSemaphores[1 - m_curComputeSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
            waitStages.push_back(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
    }
    submitInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_computeCommandBuffer.get(global::graphicsManager.getCurrentFrame());

    // Should not signal last batch, EXCEPT for existence of active swapchains
    if (batchIndex != m_passBatches.size() - 1 || !m_activeSwapchains.empty())
    {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores =
            &m_computeSyncSemaphores[m_curComputeSemphoreIndex][global::graphicsManager.getCurrentFrame()];
    }

    VkFence fence = m_computeFrameInFlightFences[global::graphicsManager.getCurrentFrame()];
    if (vkQueueSubmit(m_device.getComputeQueue(), 1, &submitInfo, fence) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to submit compute queue!");
    }

    m_curComputeSemphoreIndex = 1 - m_curComputeSemphoreIndex;
}

void VulkanContext::presentSwapchains()
{
    std::vector<VkSwapchainKHR> swapchains;
    std::vector<u32> imageIndices;
    std::vector<VkResult> results;
    for (const auto& swapchain : m_activeSwapchains)
    {
        swapchains.push_back(swapchain->get());
        imageIndices.push_back(swapchain->getImageIndex());
        results.push_back(VK_SUCCESS);
    }

    std::vector<VkSemaphore> waitSemaphores;
    if (m_passBatches.back().useGraphicsQueue)
    {
        waitSemaphores.push_back(
            m_graphicsSyncSemaphores[1 - m_curGraphicsSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
    }
    if (m_passBatches.back().useComputeQueue)
    {
        waitSemaphores.push_back(
            m_computeSyncSemaphores[1 - m_curComputeSemphoreIndex][global::graphicsManager.getCurrentFrame()]);
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = static_cast<u32>(waitSemaphores.size());
    presentInfo.pWaitSemaphores = waitSemaphores.data();
    presentInfo.swapchainCount = static_cast<u32>(swapchains.size());
    presentInfo.pSwapchains = swapchains.data();
    presentInfo.pImageIndices = imageIndices.data();
    presentInfo.pResults = results.data();

    VkResult result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)
    {
        log(LogLevel::ERR, "Failed to present swap chain images!");
    }

    u32 index = 0;
    for (const auto& swapchain : m_activeSwapchains)
    {
        swapchain->handlePresentResult(results[index++]);
    }
}

void VulkanContext::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format,
                                          VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccessMask,
                                          VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStageMask,
                                          VkPipelineStageFlags dstStageMask)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

} // namespace huedra