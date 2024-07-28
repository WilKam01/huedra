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

    std::vector<VertexInputStream> inputStreams = pipelineBuilder.getVertexInputStreams();

    std::vector<VkVertexInputBindingDescription> vertexInputBindingDescs(inputStreams.size());
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs{};

    u32 locationOffset = 0;
    for (u32 i = 0; i < static_cast<u32>(inputStreams.size()); ++i)
    {
        vertexInputBindingDescs[i].binding = i;
        vertexInputBindingDescs[i].stride = inputStreams[i].size;
        vertexInputBindingDescs[i].inputRate = convertVertexInputRate(inputStreams[i].inputRate);

        for (u32 j = 0; j < static_cast<u32>(inputStreams[i].attributes.size()); ++j)
        {
            VkVertexInputAttributeDescription attributeDescription;
            attributeDescription.location = i;
            attributeDescription.binding = locationOffset + j;
            attributeDescription.format = convertDataFormat(inputStreams[i].attributes[j].format);
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
            bindings[i][j].descriptorType = convertResourceType(resources[i][j].resource);
            bindings[i][j].stageFlags = convertShaderStage(resources[i][j].shaderStage);
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
        pushConstants[i].stageFlags = convertShaderStage(pushConstantsStages[i]);
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

VkShaderStageFlagBits VulkanPipeline::convertShaderStage(ShaderStageFlags shaderStage)
{
    PipelineType type = getBuilder().getType();
    u32 result = 0;

    switch (type)
    {
    case PipelineType::GRAPHICS:
        if ((shaderStage & HU_SHADER_STAGE_GRAPHICS_ALL) == HU_SHADER_STAGE_GRAPHICS_ALL)
        {
            return VK_SHADER_STAGE_ALL_GRAPHICS;
        }

        if (shaderStage & HU_SHADER_STAGE_VERTEX)
        {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }
        if (shaderStage & HU_SHADER_STAGE_FRAGMENT)
        {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        break;
    };

    return static_cast<VkShaderStageFlagBits>(result);
}

VkDescriptorType VulkanPipeline::convertResourceType(ResourceType resource)
{
    switch (resource)
    {
    case ResourceType::UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    case ResourceType::TEXTURE:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    default:
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    };
}

VkVertexInputRate VulkanPipeline::convertVertexInputRate(VertexInputRate inputRate)
{
    switch (inputRate)
    {
    case VertexInputRate::VERTEX:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case VertexInputRate::INSTANCE:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    }
}

VkFormat VulkanPipeline::convertDataFormat(GraphicsDataFormat format)
{
    switch (format)
    {
    case GraphicsDataFormat::R_8_INT:
        return VK_FORMAT_R8_SINT;
    case GraphicsDataFormat::R_8_UINT:
        return VK_FORMAT_R8_UINT;
    case GraphicsDataFormat::R_8_NORM:
        return VK_FORMAT_R8_SNORM;
    case GraphicsDataFormat::R_8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case GraphicsDataFormat::R_16_INT:
        return VK_FORMAT_R16_SINT;
    case GraphicsDataFormat::R_16_UINT:
        return VK_FORMAT_R16_UINT;
    case GraphicsDataFormat::R_16_FLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case GraphicsDataFormat::R_16_NORM:
        return VK_FORMAT_R16_SNORM;
    case GraphicsDataFormat::R_16_UNORM:
        return VK_FORMAT_R16_UNORM;
    case GraphicsDataFormat::R_32_INT:
        return VK_FORMAT_R32_SINT;
    case GraphicsDataFormat::R_32_UINT:
        return VK_FORMAT_R32_UINT;
    case GraphicsDataFormat::R_32_FLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case GraphicsDataFormat::R_64_INT:
        return VK_FORMAT_R64_SINT;
    case GraphicsDataFormat::R_64_UINT:
        return VK_FORMAT_R64_UINT;
    case GraphicsDataFormat::R_64_FLOAT:
        return VK_FORMAT_R64_SFLOAT;

    case GraphicsDataFormat::RG_8_INT:
        return VK_FORMAT_R8G8_SINT;
    case GraphicsDataFormat::RG_8_UINT:
        return VK_FORMAT_R8G8_UINT;
    case GraphicsDataFormat::RG_8_NORM:
        return VK_FORMAT_R8G8_SNORM;
    case GraphicsDataFormat::RG_8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case GraphicsDataFormat::RG_16_INT:
        return VK_FORMAT_R16G16_SINT;
    case GraphicsDataFormat::RG_16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case GraphicsDataFormat::RG_16_FLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case GraphicsDataFormat::RG_16_NORM:
        return VK_FORMAT_R16G16_SNORM;
    case GraphicsDataFormat::RG_16_UNORM:
        return VK_FORMAT_R16G16_UNORM;
    case GraphicsDataFormat::RG_32_INT:
        return VK_FORMAT_R32G32_SINT;
    case GraphicsDataFormat::RG_32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case GraphicsDataFormat::RG_32_FLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case GraphicsDataFormat::RG_64_INT:
        return VK_FORMAT_R64G64_SINT;
    case GraphicsDataFormat::RG_64_UINT:
        return VK_FORMAT_R64G64_UINT;
    case GraphicsDataFormat::RG_64_FLOAT:
        return VK_FORMAT_R64G64_SFLOAT;

    case GraphicsDataFormat::RGB_8_INT:
        return VK_FORMAT_R8G8B8_SINT;
    case GraphicsDataFormat::RGB_8_UINT:
        return VK_FORMAT_R8G8B8_UINT;
    case GraphicsDataFormat::RGB_8_NORM:
        return VK_FORMAT_R8G8B8_SNORM;
    case GraphicsDataFormat::RGB_8_UNORM:
        return VK_FORMAT_R8G8B8_UNORM;
    case GraphicsDataFormat::RGB_16_INT:
        return VK_FORMAT_R16G16B16_SINT;
    case GraphicsDataFormat::RGB_16_UINT:
        return VK_FORMAT_R16G16B16_UINT;
    case GraphicsDataFormat::RGB_16_FLOAT:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case GraphicsDataFormat::RGB_16_NORM:
        return VK_FORMAT_R16G16B16_SNORM;
    case GraphicsDataFormat::RGB_16_UNORM:
        return VK_FORMAT_R16G16B16_UNORM;
    case GraphicsDataFormat::RGB_32_INT:
        return VK_FORMAT_R32G32B32_SINT;
    case GraphicsDataFormat::RGB_32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case GraphicsDataFormat::RGB_32_FLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case GraphicsDataFormat::RGB_64_INT:
        return VK_FORMAT_R64G64B64_SINT;
    case GraphicsDataFormat::RGB_64_UINT:
        return VK_FORMAT_R64G64B64_UINT;
    case GraphicsDataFormat::RGB_64_FLOAT:
        return VK_FORMAT_R64G64B64_SFLOAT;

    case GraphicsDataFormat::RGBA_8_INT:
        return VK_FORMAT_R8G8B8A8_SINT;
    case GraphicsDataFormat::RGBA_8_UINT:
        return VK_FORMAT_R8G8B8A8_UINT;
    case GraphicsDataFormat::RGBA_8_NORM:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case GraphicsDataFormat::RGBA_8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case GraphicsDataFormat::RGBA_16_INT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case GraphicsDataFormat::RGBA_16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case GraphicsDataFormat::RGBA_16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case GraphicsDataFormat::RGBA_16_NORM:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case GraphicsDataFormat::RGBA_16_UNORM:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case GraphicsDataFormat::RGBA_32_INT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case GraphicsDataFormat::RGBA_32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case GraphicsDataFormat::RGBA_32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case GraphicsDataFormat::RGBA_64_INT:
        return VK_FORMAT_R64G64B64A64_SINT;
    case GraphicsDataFormat::RGBA_64_UINT:
        return VK_FORMAT_R64G64B64A64_UINT;
    case GraphicsDataFormat::RGBA_64_FLOAT:
        return VK_FORMAT_R64G64B64A64_SFLOAT;
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