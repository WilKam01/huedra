#include "loader.hpp"
#include "core/file/utils.hpp"
#include "core/memory/utils.hpp"
#include "core/serialization/base64.hpp"
#include "core/serialization/json.hpp"
#include "core/string/utils.hpp"

namespace huedra {

std::vector<MeshData> loadObj(const std::string& path)
{
    std::vector<MeshData> meshDatas;
    MeshData* curMesh = nullptr;
    std::vector<u8> bytes = readBytes(path);

    u64 lineStart = 0;
    std::vector<vec3> positions;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;
    std::map<uvec3, u32> vertices;

    auto parseLine = [&](const std::string& line) {
        std::vector<std::string> elements;
        switch (line[0])
        {
        case 'o': // Object
            curMesh = &meshDatas.emplace_back();
            curMesh->name = line.substr(2);
            positions.clear();
            normals.clear();
            uvs.clear();
            vertices.clear();
            break;
        case 'v': // Vertex
            elements = splitByChar(line, ' ');
            switch (line[1])
            {
            case 'n': // Normal
                normals.push_back(vec3(std::stof(elements[1]), std::stof(elements[2]), std::stof(elements[3])));
                break;
            case 't': // Uv
                uvs.push_back(vec2(std::stof(elements[1]),
                                   1.0f - std::stof(elements[2]))); // TODO: Add option, 1.0f - y is only Vulkan
                break;

            default: // Position
                positions.push_back(vec3(std::stof(elements[1]), std::stof(elements[2]), std::stof(elements[3])));
                break;
            }
            break;

        case 'f': // Face
            elements = splitByChar(line, ' ');
            // Triangle (3 + 'f')
            for (u64 i = 1; i < 3 + 1; ++i)
            {
                std::vector<std::string> indices = splitByChar(elements[i], '/');

                u32 uvIndex = 0;
                u32 normalIndex = 0;

                // positions and uvs, no normals
                if (indices.size() == 2) // x/y
                {
                    uvIndex = std::stoul(indices[1]);
                }
                // All or positions and normals, no uvs
                else if (indices.size() == 3) // x/y/z or x//z
                {
                    uvIndex = indices[1].empty() ? 0 : std::stoul(indices[1]);
                    normalIndex = std::stoul(indices[2]);
                }

                uvec3 vertex(std::stoul(indices[0]), uvIndex, normalIndex);
                if (!vertices.contains(vertex))
                {
                    vertices.insert(std::pair<uvec3, u32>(vertex, static_cast<u32>(vertices.size())));
                    curMesh->positions.push_back(positions[vertex[0] - 1]);
                    if (vertex[1] != 0)
                    {
                        curMesh->uvs.push_back(uvs[vertex[1] - 1]);
                    }
                    if (vertex[2] != 0)
                    {
                        curMesh->normals.push_back(normals[vertex[2] - 1]);
                    }
                }
                curMesh->indices.push_back(vertices[vertex]);
            }

            if (elements.size() == 5) // Quad (4 + 'f')
            {
                // Create another triangle
                std::vector<std::string> indices = splitByChar(elements[4], '/');

                u32 uvIndex = 0;
                u32 normalIndex = 0;

                // positions and uvs, no normals
                if (indices.size() == 2) // x/y
                {
                    uvIndex = std::stoul(indices[1]);
                }
                // All or positions and normals, no uvs
                else if (indices.size() == 3) // x/y/z or x//z
                {
                    uvIndex = indices[1].empty() ? 0 : std::stoul(indices[1]);
                    normalIndex = std::stoul(indices[2]);
                }

                uvec3 vertex(std::stoul(indices[0]), uvIndex, normalIndex);
                if (!vertices.contains(vertex))
                {
                    vertices.insert(std::pair<uvec3, u32>(vertex, static_cast<u32>(vertices.size())));
                    curMesh->positions.push_back(positions[vertex[0] - 1]);
                    if (vertex[1] != 0)
                    {
                        curMesh->uvs.push_back(uvs[vertex[1] - 1]);
                    }
                    if (vertex[2] != 0)
                    {
                        curMesh->normals.push_back(normals[vertex[2] - 1]);
                    }
                }

                // Quad (v0, v1, v2, v3) =
                // Triangles (v0, v1, v2), (v0, v2, v3)
                curMesh->indices.push_back(curMesh->indices[curMesh->indices.size() - 3]); // v0
                curMesh->indices.push_back(curMesh->indices[curMesh->indices.size() - 2]); // v2
                curMesh->indices.push_back(vertices[vertex]);
            }
            break;

        default:
            break;
        }
    };

    // Create lines
    for (u64 i = 0; i < bytes.size(); ++i)
    {
        if (static_cast<char>(bytes[i]) == '\n')
        {
            std::string line(reinterpret_cast<char*>(&bytes[lineStart]), i - lineStart);
            lineStart = i + 1;

            if (line.length() == 0)
            {
                continue;
            }

            parseLine(line);
        }
    }

    // Parse left over line
    std::string line(reinterpret_cast<char*>(&bytes[lineStart]), bytes.size() - lineStart);
    if (line.length() != 0)
    {
        parseLine(line);
    }

    if (meshDatas.empty())
    {
        log(LogLevel::WARNING, "loadObj(): {} has no mesh data", path.c_str());
    }

    return meshDatas;
}

std::vector<MeshData> loadGltf(const std::string& path, JsonObject& json,
                               const std::vector<std::vector<u8>>& byteBuffers)
{
    enum class ComponentType
    {
        INT8 = 5120,
        UINT8 = 5121,
        INT16 = 5122,
        UINT16 = 5123,
        UINT32 = 5125,
        FLOAT = 5126,
    };

    std::vector<MeshData> meshDatas;
    JsonArray& meshes = json["meshes"].asArray();
    JsonArray& accessors = json["accessors"].asArray();
    JsonArray& bufferViews = json["bufferViews"].asArray();
    JsonArray& buffers = json["buffers"].asArray();

    auto readAccessor = [&](u64 accessorIndex, const std::string& type, u32 typeCount, ComponentType componentType,
                            u32 componentTypeSize) -> std::vector<u8> {
        std::vector<u8> bytes;

        if (accessorIndex >= accessors.size() || accessors[accessorIndex].getType() != JsonValue::Type::OBJECT)
        {
            log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: accessor is incorrect or out of bounds",
                path.c_str(), accessorIndex);
            return std::vector<u8>();
        }
        JsonObject& accessor = accessors[accessorIndex].asObject();

        if (!accessor.hasMember("componentType", JsonValue::Type::UINT) ||
            accessor["componentType"].asUint() != static_cast<u32>(componentType))
        {
            log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: incorrect componentType", path.c_str(),
                accessorIndex);
            return std::vector<u8>();
        }

        if (!accessor.hasMember("type", JsonValue::Type::STRING) || accessor["type"].asString() != type)
        {
            log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: incorrect type", path.c_str(), accessorIndex);
            return std::vector<u8>();
        }

        if (!accessor.hasMember("count", JsonValue::Type::UINT))
        {
            log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: incorrect count", path.c_str(), accessorIndex);
            return std::vector<u8>();
        }
        u32 typeSize = accessor["count"].asUint() * typeCount;

        u32 accessorByteOffset =
            accessor.hasMember("byteOffset", JsonValue::Type::UINT) ? accessor["byteOffset"].asUint() : 0;
        u32 byteLen = componentTypeSize * typeSize;

        if (!accessor.hasMember("bufferView", JsonValue::Type::UINT) ||
            accessor["bufferView"].asUint() >= bufferViews.size() ||
            bufferViews[accessor["bufferView"].asUint()].getType() != JsonValue::Type::OBJECT)
        {
            log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: incorrect bufferView index/object", path.c_str(),
                accessorIndex);
            return std::vector<u8>();
        }
        JsonObject& bufferView = bufferViews[accessor["bufferView"].asUint()].asObject();

        if (!bufferView.hasMember("buffer", JsonValue::Type::UINT) || bufferView["buffer"].asUint() >= buffers.size() ||
            !bufferView.hasMember("byteLength", JsonValue::Type::UINT))
        {
            log(LogLevel::WARNING, "loadGltf(): {} with bufferView[{}]: incorrect bufferView values", path.c_str(),
                accessor["bufferView"].asUint());
            return std::vector<u8>();
        }

        const std::vector<u8>& byteBuffer = byteBuffers[bufferView["buffer"].asUint()];
        u32 viewByteLen = bufferView["byteLength"].asUint();
        u32 viewByteOffset =
            bufferView.hasMember("byteOffset", JsonValue::Type::UINT) ? bufferView["byteOffset"].asUint() : 0;
        u32 byteStride =
            bufferView.hasMember("byteStride", JsonValue::Type::UINT) ? bufferView["byteStride"].asUint() : 0;

        u32 byteOffset = accessorByteOffset + viewByteOffset;
        if (accessorByteOffset + byteLen > viewByteLen)
        {
            log(LogLevel::WARNING,
                "loadGltf(): %s with accessor[{}] and bufferView[{}]: byte range larger than view buffer "
                "byte range",
                path.c_str(), accessorIndex, accessor["bufferView"].asUint());
            return std::vector<u8>();
        }

        bytes.resize(byteLen);
        if ((std::endian::native == std::endian::little || componentType == ComponentType::INT8 ||
             componentType == ComponentType::UINT8) &&
            byteStride == 0)
        {
            std::memcpy(bytes.data(), &byteBuffer[byteOffset], byteLen);
            return bytes;
        }

        // Set byteStrid to size per component
        if (byteStride == 0)
        {
            byteStride = componentTypeSize;
        }
        for (u64 i = 0; i < typeSize; ++i)
        {
            switch (componentType)
            {
            case ComponentType::INT8:
            case ComponentType::UINT8:
                bytes[i] = byteBuffer[byteOffset + i];
                break;
            case ComponentType::INT16:
            case ComponentType::UINT16: {
                u16 value = parseFromBytes<u16>(&byteBuffer[byteOffset + i * byteStride], std::endian::little);
                bytes[i * sizeof(u16)] = static_cast<u8>(value & 0x00ff);
                bytes[i * sizeof(u16) + 1] = static_cast<u8>(value >> 8);
            }
            break;
            case ComponentType::UINT32:
            case ComponentType::FLOAT: {
                u32 value = parseFromBytes<u32>(&byteBuffer[byteOffset + i * byteStride], std::endian::little);
                bytes[i * sizeof(u32)] = static_cast<u8>((value) & 0x000000ff);
                bytes[i * sizeof(u32) + 1] = static_cast<u8>((value >> 8) & 0x000000ff);
                bytes[i * sizeof(u32) + 2] = static_cast<u8>((value >> 16) & 0x000000ff);
                bytes[i * sizeof(u32) + 3] = static_cast<u8>(value >> 24);
            }
            break;
            }
        }

        return bytes;
    };

    for (u64 i = 0; i < meshes.size(); ++i)
    {
        if (meshes[i].getType() != JsonValue::Type::OBJECT)
        {
            log(LogLevel::WARNING, "loadGltf(): {} with mesh[{}]: not an object", path.c_str(), i);
            return std::vector<MeshData>();
        }
        JsonObject& mesh = meshes[i].asObject();

        if (!mesh.hasMember("primitives", JsonValue::Type::ARRAY))
        {
            log(LogLevel::WARNING, "loadGltf(): {} with mesh[{}]: incorrect primitives member", path.c_str(), i);
            return std::vector<MeshData>();
        }
        JsonArray& primitives = mesh["primitives"].asArray();
        MeshData& meshData = meshDatas.emplace_back();

        for (u64 j = 0; j < primitives.size(); ++j)
        {
            if (primitives[j].getType() != JsonValue::Type::OBJECT)
            {
                log(LogLevel::WARNING, "loadGltf(): {} with mesh[{}].primitives[{}]: not an object", path.c_str(), i,
                    j);
                return std::vector<MeshData>();
            }
            JsonObject& primitive = primitives[j].asObject();

            if (!primitive.hasMember("attributes", JsonValue::Type::OBJECT))
            {
                log(LogLevel::WARNING, "loadGltf(): {} with mesh[{}].primitives[{}]: incorrect attributes member",
                    path.c_str(), i, j);
                return std::vector<MeshData>();
            }
            JsonObject& attribute = primitive["attributes"].asObject();

            if (!attribute.hasMember("POSITION", JsonValue::Type::UINT) ||
                !primitive.hasMember("indices", JsonValue::Type::UINT))
            {
                log(LogLevel::WARNING, "loadGltf(): {} with mesh[{}].primitives[{}]: incorrect vertex data",
                    path.c_str(), i, j);
                return std::vector<MeshData>();
            }

            std::vector<u8> bytes = readAccessor(attribute["POSITION"].asUint(), "VEC3", 3, ComponentType::FLOAT, 4);
            meshData.positions = std::vector<vec3>(reinterpret_cast<vec3*>(bytes.data()),
                                                   reinterpret_cast<vec3*>(bytes.data() + bytes.size()));

            if (primitive["indices"].asUint() >= accessors.size() ||
                accessors[primitive["indices"].asUint()].getType() != JsonValue::Type::OBJECT)
            {
                log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: accessor is incorrect or out of bounds",
                    path.c_str(), primitive["indices"].asUint());
                return std::vector<MeshData>();
            }
            JsonObject& accessor = accessors[primitive["indices"].asUint()].asObject();

            if (!accessor.hasMember("componentType", JsonValue::Type::UINT) ||
                (accessor["componentType"].asUint() != static_cast<u32>(ComponentType::UINT8) &&
                 accessor["componentType"].asUint() != static_cast<u32>(ComponentType::UINT16) &&
                 accessor["componentType"].asUint() != static_cast<u32>(ComponentType::UINT32)))
            {
                log(LogLevel::WARNING, "loadGltf(): {} with accessor[{}]: incorrect componentType", path.c_str(),
                    primitive["indices"].asUint());
                return std::vector<MeshData>();
            }
            ComponentType indexType = static_cast<ComponentType>(accessor["componentType"].asUint());

            if (indexType == ComponentType::UINT8)
            {
                bytes = readAccessor(primitive["indices"].asUint(), "SCALAR", 1, ComponentType::UINT8, 1);
                meshData.indices = std::vector<u32>(bytes.begin(), bytes.end());
            }
            else if (indexType == ComponentType::UINT16)
            {
                bytes = readAccessor(primitive["indices"].asUint(), "SCALAR", 1, ComponentType::UINT16, 2);
                std::vector<u16> bytes16 = std::vector<u16>(reinterpret_cast<u16*>(bytes.data()),
                                                            reinterpret_cast<u16*>(bytes.data() + bytes.size()));
                meshData.indices = std::vector<u32>(bytes16.begin(), bytes16.end());
            }
            else if (indexType == ComponentType::UINT32)
            {
                bytes = readAccessor(primitive["indices"].asUint(), "SCALAR", 1, ComponentType::UINT32, 4);
                meshData.indices = std::vector<u32>(reinterpret_cast<u32*>(bytes.data()),
                                                    reinterpret_cast<u32*>(bytes.data() + bytes.size()));
            }

            if (attribute.hasMember("NORMAL", JsonValue::Type::UINT))
            {
                bytes = readAccessor(attribute["NORMAL"].asUint(), "VEC3", 3, ComponentType::FLOAT, 4);
                meshData.normals = std::vector<vec3>(reinterpret_cast<vec3*>(bytes.data()),
                                                     reinterpret_cast<vec3*>(bytes.data() + bytes.size()));
            }
            if (attribute.hasMember("TEXCOORD_0", JsonValue::Type::UINT))
            {
                bytes = readAccessor(attribute["TEXCOORD_0"].asUint(), "VEC2", 2, ComponentType::FLOAT, 4);
                meshData.uvs = std::vector<vec2>(reinterpret_cast<vec2*>(bytes.data()),
                                                 reinterpret_cast<vec2*>(bytes.data() + bytes.size()));
            }
        }
    }

    return meshDatas;
}

std::vector<MeshData> loadGltf(const std::string& path)
{
    enum class ComponentType
    {
        INT8 = 5120,
        UINT8 = 5121,
        INT16 = 5122,
        UINT16 = 5123,
        UINT32 = 5125,
        FLOAT = 5126,
    };

    JsonObject json = parseJson(readBytes(path));
    std::string relPath = splitLastByChar(path, '/')[0] + "/";

    if (!json.hasMember("meshes", JsonValue::Type::ARRAY) || !json.hasMember("accessors", JsonValue::Type::ARRAY) ||
        !json.hasMember("bufferViews", JsonValue::Type::ARRAY) || !json.hasMember("buffers", JsonValue::Type::ARRAY))
    {
        log(LogLevel::WARNING, "loadGltf(): {} has incorrect mesh data", path.c_str());
        return std::vector<MeshData>();
    }
    JsonArray& buffers = json["buffers"].asArray();

    std::vector<std::vector<u8>> byteBuffers(buffers.size());
    for (u64 i = 0; i < buffers.size(); ++i)
    {
        if (buffers[i].getType() != JsonValue::Type::OBJECT)
        {
            log(LogLevel::WARNING, "loadGltf(): {} with buffer[{}]: not an object", path.c_str(), i);
            return std::vector<MeshData>();
        }
        JsonObject& buffer = buffers[i].asObject();

        if (!buffer.hasMember("byteLength", JsonValue::Type::UINT))
        {
            log(LogLevel::WARNING, "loadGltf(): {} with buffer[{}]: incorrect byteLength", path.c_str(), i);
            return std::vector<MeshData>();
        }

        if (buffer.hasMember("uri", JsonValue::Type::STRING))
        {
            std::string& uri = buffer["uri"].asString();
            if (uri.starts_with("data:"))
            {
                std::string type = splitByChar(uri.substr(5), ';')[0];
                // TODO: Check support?
                byteBuffers[i] = decodeBase64(splitByChar(uri, ',')[1]);
            }
            // Path
            else
            {
                byteBuffers[i] = readBytes(relPath + uri);
            }
        }
        else
        {
            // Transforms path to bin extension, ex: "dir1/dir2/file.gltf" -> "dir1/dir2/file.bin"
            byteBuffers[i] = readBytes(splitByChar(path, '.')[0] + ".bin");
        }
    }

    return loadGltf(path, json, byteBuffers);
}

std::vector<MeshData> loadGlb(const std::string& path)
{
    std::vector<MeshData> meshData;
    std::vector<u8> bytes = readBytes(path);

    const u32 gltfSignature = 0x46546c67; // "glTF"
    const u32 jsonSignature = 0x4e4f534a; // "JSON"
    const u32 binSignature = 0x004e4942;  // "\0BIN"

    if (parseFromBytes<u32>(&bytes[0], std::endian::little) != gltfSignature)
    {
        log(LogLevel::WARNING, "loadGlb(): {} incorrect magic number: {}, expected {}", path.c_str(),
            parseFromBytes<u32>(&bytes[0], std::endian::little), gltfSignature);
        return std::vector<MeshData>();
    }

    u32 byteIndex = 12;
    u32 chunkIndex = 0;

    JsonObject json;
    std::vector<std::vector<u8>> byteBuffers;
    bool foundJson = false;

    while (byteIndex < bytes.size())
    {
        u32 chunkLen = parseFromBytes<u32>(&bytes[byteIndex], std::endian::little);
        u32 chunkType = parseFromBytes<u32>(&bytes[byteIndex + 4], std::endian::little);
        byteIndex += 8;

        if (chunkType == jsonSignature)
        {
            if (foundJson)
            {
                log(LogLevel::WARNING, "loadGlb(): {} chunk[{}]: duplicate json data", path.c_str(), chunkIndex);
                return std::vector<MeshData>();
            }
            json = parseJson(std::vector<u8>(bytes.begin() + byteIndex, bytes.begin() + byteIndex + chunkLen));
            foundJson = true;
        }
        else if (chunkType == binSignature)
        {
            byteBuffers.push_back(std::vector<u8>(bytes.begin() + byteIndex, bytes.begin() + byteIndex + chunkLen));
        }
        else
        {
            log(LogLevel::WARNING, "loadGlb(): {} chunk[{}]: incorrect chunkType: {}", path.c_str(), chunkIndex,
                chunkType);
            return std::vector<MeshData>();
        }

        byteIndex += chunkLen;
        ++chunkIndex;
    }

    if (!json.hasMember("meshes", JsonValue::Type::ARRAY) || !json.hasMember("accessors", JsonValue::Type::ARRAY) ||
        !json.hasMember("bufferViews", JsonValue::Type::ARRAY) || !json.hasMember("buffers", JsonValue::Type::ARRAY) ||
        byteBuffers.size() != json["buffers"].asArray().size())
    {
        log(LogLevel::WARNING, "loadGlb(): {} has incorrect mesh data", path.c_str());
        return std::vector<MeshData>();
    }

    return loadGltf(path, json, byteBuffers);
}

} // namespace huedra