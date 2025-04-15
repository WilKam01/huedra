#include "resource_manager.hpp"
#include "core/file/utils.hpp"
#include "core/global.hpp"
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
        FilePathInfo info = transformFilePath(path);
        if (info.extension == "obj")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadObj(path)));
        }
        else if (info.extension == "gltf")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadGltf(path)));
        }
        else if (info.extension == "glb")
        {
            m_meshDatas.insert(std::pair<u64, std::vector<MeshData>>(hash, loadGlb(path)));
        }
        else
        {
            log(LogLevel::WARNING, "loadMeshData(): extension \"{}\" not supported", info.extension.c_str());
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
        FilePathInfo info = transformFilePath(path);
        if (info.extension == "png")
        {
            m_textureDatas.insert(std::pair<u64, TextureData>(hash, loadPng(path, channelFormat)));
        }
        else
        {
            log(LogLevel::WARNING, "loadTextureData(): extension \"{}\" not supported", info.extension.c_str());
            return m_missingTextureData;
        }
    }
    return m_textureDatas[hash];
}

ShaderModule& ResourceManager::loadShaderModule(const std::string& path)
{
    u64 hash = m_strHash(path);
    if (!m_shaders.contains(hash))
    {
        FilePathInfo info = transformFilePath(path);
        if (info.extension != "slang")
        {
            log(LogLevel::ERR, "loadShaderModule(): extension {} not supported", info.extension.c_str());
            return m_missingShaderModule;
        }

        std::vector<u8> bytes = readBytes(path);
        ShaderModule shaderModule =
            global::graphicsManager.createShaderModule(info.fileName, bytes.data(), bytes.size());
        if (shaderModule.getSlangModule() == nullptr)
        {
            log(LogLevel::ERR, "loadShaderModule(): Could not create shader module");
        }
        m_shaders.insert(std::pair<u64, ShaderModule>(hash, shaderModule));
    }
    return m_shaders[hash];
}

} // namespace huedra