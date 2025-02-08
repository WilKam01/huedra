#pragma once

#include "core/types.hpp"
#include "resources/mesh/data.hpp"
#include "resources/texture/data.hpp"

namespace huedra {

class ResourceManager
{
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    void init();
    void cleanup();

    std::vector<MeshData>& loadMeshData(const std::string& path);
    TextureData& loadTextureData(const std::string& path, TexelChannelFormat channelFormat);

private:
    std::hash<std::string> m_strHash;
    std::vector<MeshData> m_missingMeshData;
    TextureData m_missingTextureData;

    std::unordered_map<u64, std::vector<MeshData>> m_meshDatas;
    std::unordered_map<u64, TextureData> m_textureDatas;
};

} // namespace huedra