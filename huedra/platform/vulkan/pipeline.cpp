#include "pipeline.hpp"
#include "core/file/utils.hpp"
#include "core/global.hpp"
#include "core/log.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass,
                                  u32 targetCount)
{
    m_device = &device;
    m_builder = pipelineBuilder;

    std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    std::map<ShaderStage, ShaderInput> shaders = pipelineBuilder.getShaderStages();
    m_shaderModule = global::graphicsManager.compileAndLinkShaderModules(shaders);

    initLayout();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = static_cast<u32>(m_shaderModule.getCode().size());
    createInfo.pCode = reinterpret_cast<const u32*>(m_shaderModule.getCode().data());

    VkShaderModule shaderModule{nullptr};
    if (vkCreateShaderModule(m_device->getLogical(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create shader module!");
    }

    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = shaders[ShaderStage::VERTEX].entryPointName.c_str();
    shaderCreateInfos.push_back(shaderStageInfo);

    if (shaders.contains(ShaderStage::FRAGMENT))
    {
        shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfo.module = shaderModule;
        shaderStageInfo.pName = shaders[ShaderStage::FRAGMENT].entryPointName.c_str();
        shaderCreateInfos.push_back(shaderStageInfo);
    }

    std::vector<VertexInputStream> inputStreams = pipelineBuilder.getVertexInputStreams();

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs(inputStreams.size());
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs{};

    u32 locationOffset = 0;
    for (u32 i = 0; i < static_cast<u32>(inputStreams.size()); ++i)
    {
        vertexInputBindingDescs[i].binding = i;
        vertexInputBindingDescs[i].stride = inputStreams[i].size;
        vertexInputBindingDescs[i].inputRate = converter::convertVertexInputRate(inputStreams[i].inputRate);

        for (u32 j = 0; j < static_cast<u32>(inputStreams[i].attributes.size()); ++j)
        {
            VkVertexInputAttributeDescription attributeDescription;
            attributeDescription.binding = i;
            attributeDescription.location = locationOffset + j;
            attributeDescription.format = converter::convertDataFormat(inputStreams[i].attributes[j].format);
            attributeDescription.offset = inputStreams[i].attributes[j].offset;
            vertexInputAttributeDescs.push_back(attributeDescription);
        }

        locationOffset += static_cast<u32>(inputStreams[i].attributes.size());
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(vertexInputBindingDescs.size());
    vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescs.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(vertexInputAttributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescs.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = convertPrimitiveTopology(m_builder.getPrimitiveLayout());
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = convertPolygonMode(m_builder.getPrimitiveType());
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = 0.2f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(targetCount, colorBlendAttachment);

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = targetCount;
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<u32>(shaderCreateInfos.size());
    pipelineInfo.pStages = shaderCreateInfos.data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(m_device->getLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) !=
        VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device->getLogical(), shaderModule, nullptr);
}

void VulkanPipeline::initCompute(const PipelineBuilder& pipelineBuilder, Device& device)
{
    m_device = &device;
    m_builder = pipelineBuilder;

    std::map<ShaderStage, ShaderInput> shaders = pipelineBuilder.getShaderStages();
    m_shaderModule = global::graphicsManager.compileAndLinkShaderModules(shaders);

    initLayout();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = static_cast<u32>(m_shaderModule.getCode().size());
    createInfo.pCode = reinterpret_cast<const u32*>(m_shaderModule.getCode().data());

    VkShaderModule shaderModule{nullptr};
    if (vkCreateShaderModule(m_device->getLogical(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create shader module!");
    }

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.pName = shaders[ShaderStage::COMPUTE].entryPointName.c_str();
    shaderStageInfo.module = shaderModule;

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.stage = shaderStageInfo;

    if (vkCreateComputePipelines(m_device->getLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) !=
        VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(m_device->getLogical(), shaderModule, nullptr);
}

void VulkanPipeline::cleanup()
{
    vkDestroyPipeline(m_device->getLogical(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device->getLogical(), m_pipelineLayout, nullptr);

    for (auto& descriptorLayout : m_descriptorLayout)
    {
        vkDestroyDescriptorSetLayout(m_device->getLogical(), descriptorLayout, nullptr);
    }
}

void VulkanPipeline::initLayout()
{
    PipelineBuilder& builder = getBuilder();

    std::vector<std::vector<ResourceBinding>> resources = m_shaderModule.getResources();
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> sets(resources.size());
    m_descriptorLayout.resize(resources.size());

    for (u64 i = 0; i < resources.size(); ++i)
    {
        sets[i].resize(resources[i].size());
        for (u64 j = 0; j < resources[i].size(); ++j)
        {
            sets[i][j].binding = j;
            sets[i][j].descriptorType = converter::convertResourceType(resources[i][j].type);
            sets[i][j].stageFlags = converter::convertShaderStage(builder.getType(), resources[i][j].shaderStage);
            sets[i][j].descriptorCount = 1; // TODO: Support binding arrays
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<u32>(sets[i].size());
        layoutInfo.pBindings = sets[i].data();

        if (vkCreateDescriptorSetLayout(m_device->getLogical(), &layoutInfo, nullptr, &m_descriptorLayout[i]) !=
            VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create descriptor set layout!");
        }
    }

    std::vector<ParameterBinding> parameterBinding = m_shaderModule.getParameters();
    std::vector<VkPushConstantRange> pushConstants(parameterBinding.size());

    for (u64 i = 0; i < pushConstants.size(); ++i)
    {
        pushConstants[i].stageFlags = converter::convertShaderStage(builder.getType(), parameterBinding[i].shaderStage);
        pushConstants[i].offset = parameterBinding[i].offset;
        pushConstants[i].size = parameterBinding[i].size;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (!m_descriptorLayout.empty())
    {
        pipelineLayoutInfo.setLayoutCount = static_cast<u32>(m_descriptorLayout.size());
        pipelineLayoutInfo.pSetLayouts = m_descriptorLayout.data();
    }

    if (!pushConstants.empty())
    {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<u32>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    }

    if (vkCreatePipelineLayout(m_device->getLogical(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create pipeline layout!");
    }
}

VkPolygonMode VulkanPipeline::convertPolygonMode(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::POINT:
        return VK_POLYGON_MODE_POINT;
    case PrimitiveType::LINE:
        return VK_POLYGON_MODE_LINE;
    case PrimitiveType::TRIANGLE:
        return VK_POLYGON_MODE_FILL;
    defualt:
        return VK_POLYGON_MODE_FILL;
    }
    return VK_POLYGON_MODE_FILL;
}

VkPrimitiveTopology VulkanPipeline::convertPrimitiveTopology(PrimitiveLayout layout)
{
    switch (layout)
    {
    case PrimitiveLayout::POINT_LIST:
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case PrimitiveLayout::LINE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case PrimitiveLayout::LINE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case PrimitiveLayout::TRIANGLE_LIST:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case PrimitiveLayout::TRIANGLE_STRIP:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    defualt:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

} // namespace huedra