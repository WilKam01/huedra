#pragma once

#include "core/types.hpp"
#include "graphics/shader_module.hpp"
#include "resources/mesh/data.hpp"
#include "resources/texture/data.hpp"

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

    std::vector<MeshData>& loadMeshData(const std::string& path);
    TextureData& loadTextureData(const std::string& path, TexelChannelFormat channelFormat);
    ShaderModule& loadShaderModule(const std::string& path);

private:
    std::hash<std::string> m_strHash;

    std::vector<MeshData> m_missingMeshData;
    TextureData m_missingTextureData;
    ShaderModule m_missingShaderModule;

    std::unordered_map<u64, std::vector<MeshData>> m_meshDatas;
    std::unordered_map<u64, TextureData> m_textureDatas;
    std::unordered_map<u64, ShaderModule> m_shaders;
};

} // namespace huedra