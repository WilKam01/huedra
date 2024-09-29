#include "loader.hpp"
#include "core/file/utils.hpp"
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
        log(LogLevel::ERR, "loadObj(): %s has no mesh data", path.c_str());
    }

    return meshDatas;
}

} // namespace huedra