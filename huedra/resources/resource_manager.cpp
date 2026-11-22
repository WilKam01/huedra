#include "resource_manager.hpp"
#include "huedra/core/constants.hpp"
#include "huedra/core/file/utils.hpp"
#include "huedra/core/global.hpp"
#include "huedra/core/log.hpp"
#include "huedra/core/string/utils.hpp"
#include "huedra/resources/mesh/loader.hpp"
#include "huedra/resources/texture/loader.hpp"

namespace huedra {

void ResourceManager::init() {}

void ResourceManager::cleanup()
{
    m_meshDatas.clear();
    m_textureDatas.clear();
}

std::vector<MeshData>& ResourceManager::loadMeshData(std::string_view path)
{
    u64 hash = constants::FNV_OFFSET;
    auto combineHash = [&hash](u64 val) { hash ^= val * constants::FNV_PRIME; };
    auto u64Hash = std::hash<u64>();
    for (const auto& c : path)
    {
        combineHash(u64Hash(static_cast<u64>(c)));
    }

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
            log::func::warn("Extension \"{}\" not supported", info.extension.c_str());
            return m_missingMeshData;
        }
    }
    return m_meshDatas[hash];
}

TextureData& ResourceManager::loadTextureData(std::string_view path, TexelChannelFormat channelFormat)
{
    u64 hash = constants::FNV_OFFSET;
    auto combineHash = [&hash](u64 val) { hash ^= val * constants::FNV_PRIME; };
    auto u64Hash = std::hash<u64>();
    for (const auto& c : path)
    {
        combineHash(u64Hash(static_cast<u64>(c)));
    }

    if (!m_textureDatas.contains(hash))
    {
        FilePathInfo info = transformFilePath(path);
        if (info.extension == "png")
        {
            m_textureDatas.insert(std::pair<u64, TextureData>(hash, loadPng(path, channelFormat)));
        }
        else
        {
            log::func::warn("Extension \"{}\" not supported", info.extension.c_str());
            return m_missingTextureData;
        }
    }
    return m_textureDatas[hash];
}

HdrTextureData& ResourceManager::loadHdrTextureData(std::string_view path, TexelChannelFormat channelFormat)
{
    u64 hash = constants::FNV_OFFSET;
    auto combineHash = [&hash](u64 val) { hash ^= val * constants::FNV_PRIME; };
    auto u64Hash = std::hash<u64>();
    for (const auto& c : path)
    {
        combineHash(u64Hash(static_cast<u64>(c)));
    }

    if (!m_hdrTextureDatas.contains(hash))
    {
        FilePathInfo info = transformFilePath(path);
        if (info.extension == "hdr")
        {
            m_hdrTextureDatas.insert(std::pair<u64, HdrTextureData>(hash, loadRadianceHdr(path, channelFormat)));
        }
        else
        {
            log::func::warn("Extension \"{}\" not supported", info.extension.c_str());
            return m_missingHdrTextureData;
        }
    }
    return m_hdrTextureDatas[hash];
}

ShaderModule& ResourceManager::loadShaderModule(std::string_view path)
{
    u64 hash = constants::FNV_OFFSET;
    auto combineHash = [&hash](u64 val) { hash ^= val * constants::FNV_PRIME; };
    auto u64Hash = std::hash<u64>();
    for (const auto& c : path)
    {
        combineHash(u64Hash(static_cast<u64>(c)));
    }

    if (!m_shaders.contains(hash))
    {
        FilePathInfo info = transformFilePath(path);
        if (info.extension != "slang")
        {
            log::func::fatal("Extension {} not supported", info.extension.c_str());
            return m_missingShaderModule;
        }

        std::vector<u8> bytes = readBytes(path);
        if (bytes.empty())
        {
            return m_missingShaderModule;
        }

        ShaderModule shaderModule =
            global::graphicsManager.createShaderModule(info.dirPath + "/" + info.fileName, bytes.data(), bytes.size());
        if (shaderModule.getSlangModule() == nullptr)
        {
            log::func::fatal("Could not create shader module");
            return m_missingShaderModule;
        }
        m_shaders.insert(std::pair<u64, ShaderModule>(hash, shaderModule));
    }
    return m_shaders[hash];
}

} // namespace huedra