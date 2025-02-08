#include "resource_manager.hpp"
#include "core/log.hpp"
#include "core/string/utils.hpp"
#include "resources/mesh/loader.hpp"
#include "resources/texture/loader.hpp"

namespace huedra {

void ResourceManager::init() {}

void ResourceManager::cleanup()
{
    m_meshDatas.clear();
    m_textureDatas.clear();
}

std::vector<MeshData>& ResourceManager::loadMeshData(const std::string& path)
{
    u64 hash = m_strHash(path);
    if (!m_meshDatas.contains(hash))
    {
        std::string extension = splitLastByChar(path, '.').back();
        if (extension == "obj")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadObj(path)));
        }
        else if (extension == "gltf")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadGltf(path)));
        }
        else if (extension == "glb")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadGlb(path)));
        }
        else
        {
            log(LogLevel::WARNING, "loadMeshData(): extension %s not supported", extension.c_str());
            return m_missingMeshData;
        }
    }
    return m_meshDatas[hash];
}

TextureData& ResourceManager::loadTextureData(const std::string& path, TexelChannelFormat channelFormat)
{
    u64 hash = m_strHash(path);
    if (!m_textureDatas.contains(hash))
    {
        std::string extension = splitLastByChar(path, '.').back();
        if (extension == "png")
        {
            m_textureDatas.insert(std::pair<u64, TextureData>(hash, loadPng(path, channelFormat)));
        }
        else
        {
            log(LogLevel::WARNING, "loadTextureData(): extension %s not supported", extension.c_str());
            return m_missingTextureData;
        }
    }
    return m_textureDatas[hash];
}

} // namespace huedra