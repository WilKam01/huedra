#include "pipeline.hpp"
#include "core/file/utils.hpp"
#include "core/log.hpp"
#include "platform/vulkan/type_converter.hpp"

namespace huedra {

void VulkanPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass)
{
    p_device = &device;
    m_builder = pipelineBuilder;
    initLayout();

    std::map<ShaderStage, std::string> shaders = pipelineBuilder.getShaderStages();
    std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
    std::vector<VkShaderModule> shaderModules{};

    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfo.pName = "main";

    shaderModules.push_back(loadShader(shaders[ShaderStage::VERTEX]));
    shaderStageInfo.module = shaderModules.back();
    shaderCreateInfos.push_back(shaderStageInfo);

    if (shaders.contains(ShaderStage::FRAGMENT))
    {
        shaderModules.push_back(loadShader(shaders[ShaderStage::FRAGMENT]));
        shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageInfo.module = shaderModules.back();
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
            attributeDescription.location = i;
            attributeDescription.binding = locationOffset + j;
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
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // TODO: Msaa
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = .2f;
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

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
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

    if (vkCreateGraphicsPipelines(p_device->getLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) !=
        VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create graphics pipeline!");
    }

    for (auto& module : shaderModules)
    {
        vkDestroyShaderModule(p_device->getLogical(), module, nullptr);
    }
}

void VulkanPipeline::initCompute(const PipelineBuilder& pipelineBuilder, Device& device)
{
    p_device = &device;
    m_builder = pipelineBuilder;
    initLayout();

    VkShaderModule shaderModule{loadShader(pipelineBuilder.getShaderStages()[ShaderStage::COMPUTE])};
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageInfo.pName = "main";
    shaderStageInfo.module = shaderModule;

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.stage = shaderStageInfo;

    if (vkCreateComputePipelines(p_device->getLogical(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) !=
        VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(p_device->getLogical(), shaderModule, nullptr);
}

void VulkanPipeline::cleanup()
{
    vkDestroyPipeline(p_device->getLogical(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(p_device->getLogical(), m_pipelineLayout, nullptr);

    for (auto& descriptorLayout : m_descriptorLayout)
    {
        vkDestroyDescriptorSetLayout(p_device->getLogical(), descriptorLayout, nullptr);
    }
}

void VulkanPipeline::initLayout()
{
    PipelineBuilder& builder = getBuilder();

    std::vector<std::vector<ResourceBinding>> resources = builder.getResources();
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings(resources.size());
    m_descriptorLayout.resize(resources.size());

    for (size_t i = 0; i < resources.size(); ++i)
    {
        bindings[i].resize(resources[i].size());
        for (size_t j = 0; j < resources[i].size(); ++j)
        {
            bindings[i][j].binding = j;
            bindings[i][j].descriptorType = converter::convertResourceType(resources[i][j].resource);
            bindings[i][j].stageFlags = converter::convertShaderStage(builder.getType(), resources[i][j].shaderStage);
            bindings[i][j].descriptorCount = 1;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<u32>(bindings[i].size());
        layoutInfo.pBindings = bindings[i].data();

        if (vkCreateDescriptorSetLayout(p_device->getLogical(), &layoutInfo, nullptr, &m_descriptorLayout[i]) !=
            VK_SUCCESS)
        {
            log(LogLevel::ERR, "Failed to create descriptor set layout!");
        }
    }

    std::vector<u32> pushConstantRanges = builder.getPushConstantRanges();
    std::vector<ShaderStageFlags> pushConstantsStages = builder.getPushConstantShaderStages();
    std::vector<VkPushConstantRange> pushConstants(pushConstantRanges.size());

    u32 pushConstantOffset = 0;
    for (size_t i = 0; i < pushConstants.size(); ++i)
    {
        pushConstants[i].stageFlags = converter::convertShaderStage(builder.getType(), pushConstantsStages[i]);
        pushConstants[i].offset = pushConstantOffset;
        pushConstants[i].size = pushConstantRanges[i];

        pushConstantOffset += pushConstantRanges[i];
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

    if (vkCreatePipelineLayout(p_device->getLogical(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create pipeline layout!");
    }
}

VkShaderModule VulkanPipeline::loadShader(const std::string& path)
{
    std::vector<u8> buffer = readBytes(path + ".spv");

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const u32*>(buffer.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(p_device->getLogical(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create shader module!");
    }

    return shaderModule;
}

} // namespace huedra