#pragma once

#include "huedra/core/types.hpp"
#include "huedra/graphics/shader_module.hpp"
#include "huedra/resources/mesh/data.hpp"
#include "huedra/resources/texture/data.hpp"

namespace huedra {

class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager& rhs) = default;
    ResourceManager& operator=(const ResourceManager& rhs) = default;
    ResourceManager(ResourceManager&& rhs) = default;
    ResourceManager& operator=(ResourceManager&& rhs) = default;

    void init();
    void cleanup();

    std::vector<MeshData>& loadMeshData(std::string_view path);
    TextureData& loadTextureData(std::string_view path, TexelChannelFormat channelFormat);
    HdrTextureData& loadHdrTextureData(std::string_view path, TexelChannelFormat channelFormat);
    ShaderModule& loadShaderModule(std::string_view path);

private:
    std::vector<MeshData> m_missingMeshData;
    TextureData m_missingTextureData;
    HdrTextureData m_missingHdrTextureData;
    ShaderModule m_missingShaderModule;

    std::unordered_map<u64, std::vector<MeshData>> m_meshDatas;
    std::unordered_map<u64, TextureData> m_textureDatas;
    std::unordered_map<u64, HdrTextureData> m_hdrTextureDatas;
    std::unordered_map<u64, ShaderModule> m_shaders;
};

} // namespace huedra