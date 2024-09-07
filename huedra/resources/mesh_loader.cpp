#include "mesh_loader.hpp"
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
                uvs.push_back(vec2(std::stof(elements[1]), std::stof(elements[2])));
                break;

            default: // Position
                positions.push_back(vec3(std::stof(elements[1]), std::stof(elements[2]), std::stof(elements[3])));
                break;
            }
            break;

        case 'f': // Face
            elements = splitByChar(line, ' ');
            for (u64 i = 1; i < elements.size(); ++i)
            {
                std::vector<std::string> indices = splitByChar(elements[i], '/');
                uvec3 vertex(std::stoul(indices[0]), std::stoul(indices[1]), std::stoul(indices[2]));

                if (!vertices.contains(vertex))
                {
                    vertices.insert(std::pair<uvec3, u32>(vertex, static_cast<u32>(vertices.size())));
                    curMesh->positions.push_back(positions[vertex[0] - 1]);
                    curMesh->uvs.push_back(uvs[vertex[1] - 1]);
                    curMesh->normals.push_back(normals[vertex[2] - 1]);
                }
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

    return meshDatas;
}

} // namespace huedra