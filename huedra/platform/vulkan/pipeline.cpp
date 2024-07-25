#include "pipeline.hpp"
#include "core/log.hpp"

#include <fstream>

namespace huedra {

void VulkanPipeline::initGraphics(const PipelineBuilder& pipelineBuilder, Device& device, VkRenderPass renderPass)
{
    Pipeline::init(pipelineBuilder);
    p_device = &device;
    initLayout();

    std::map<ShaderStage, std::string> shaders = pipelineBuilder.getShaderStages();
    if (!shaders.contains(ShaderStage::VERTEX))
    {
        log(LogLevel::ERR, "Could not create graphics pipeline, vertex shader not present!");
    }

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

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs{};
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs{};

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
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<u32>(shaderCreateInfos.size());
    pipelineInfo.pStages = shaderCreateInfos.data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_layout;
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

void VulkanPipeline::cleanup()
{
    vkDestroyPipeline(p_device->getLogical(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(p_device->getLogical(), m_layout, nullptr);
}

void VulkanPipeline::initLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(p_device->getLogical(), &pipelineLayoutInfo, nullptr, &m_layout) != VK_SUCCESS)
    {
        log(LogLevel::ERR, "Failed to create pipeline layout!");
    }
}

VkShaderModule VulkanPipeline::loadShader(const std::string& path)
{
    std::string realPath = path + ".spv";
    std::ifstream file(realPath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        log(LogLevel::ERR, "Failed to open file: \"%s\"!", realPath.c_str());
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

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