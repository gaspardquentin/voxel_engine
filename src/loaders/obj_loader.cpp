#include "loaders/obj_loader.h"
#include "rendering/mesh_data.h"
#include "voxel_engine/math_utils.h"
#include <cstdint>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static void parseVec3(std::istringstream& stream, std::vector<Vec3>& vec) {
    std::string token;
    std::vector<float> values;
    while (stream >> token) {
        values.push_back(std::stof(token));
    }

    if (values.size() < 2) {
        std::cerr << "<voxeng> error: incorrect obj file line (not enough values)\n"; 
        return;
    }

    // case for texture coordinates (2 values) vs position/normals (3 values)
    if (values.size() == 2) {
        vec.push_back({values[0], values[1], 0.0f});
    } else {
        vec.push_back({values[0], values[1], values[2]});
    }
}

static bool processFace(std::istringstream& line_stream, 
                        const std::vector<Vec3>& positions,
                        const std::vector<Vec3>& normals,
                        const std::vector<Vec3>& uvs,
                        std::unordered_map<ObjVertexIndex, uint32_t>& unique_vertices,
                        MeshData& mesh) {
    std::string token;
    int i = 0;
    while (line_stream >> token) {
        if (i >= 3) {
            std::cerr << "obj_parser: Quads are not supported yet.\n";
            //TODO: support quads by turning it to two triangles
            return false;
        }

        std::array<std::optional<int>, 3> vals = {};
        std::stringstream token_stream(token);
        std::string item;
        int j = 0;
        // Parse "v/vt/vn"
        while (std::getline(token_stream, item, '/')) {
            if (j >= 3) {
                std::cerr << "faces must have a maximum of 3 parameters\n";
                return false;
            }
            if (!item.empty()) {
                vals[j] = std::stoi(item);
            }
            j++;
        }

        if (!vals[0].has_value()) {
            std::cerr << "faces needs at least a position value\n";
            return false;
        }

        // Position
        int v_idx = vals[0].value() - 1;
        if (v_idx < 0 || static_cast<size_t>(v_idx) >= positions.size()) {
            std::cerr << "face position index out of bounds.\n";
            return false;
        }

        // Normal
        Vec3 normal = {0.0f, 0.0f, 0.0f};
        int vn_idx = 0;
        if (vals[2].has_value()) {
            vn_idx = vals[2].value() - 1;
            if (vn_idx < 0 || static_cast<size_t>(vn_idx) >= normals.size()) {
                std::cerr << "face normal index out of bounds.\n";
                return false;
            }
            normal = normals[vn_idx];
        }

        // UV
        Vec3 uv = {0.0f, 0.0f, 0.0f};
        int vt_idx = 0;
        if (vals[1].has_value()) {
            vt_idx = vals[1].value() - 1;
            if (vt_idx < 0 || static_cast<size_t>(vt_idx) >= uvs.size()) {
                std::cerr << "face uv index out of bounds.\n";
                return false;
            }
            uv = uvs[vt_idx];
        }

        // Indexing (de-duplication of vertices)
        ObjVertexIndex key = {v_idx, vt_idx, vn_idx};

        if (unique_vertices.count(key)) {
            // Re-use existing vertex
            mesh.indices.push_back(unique_vertices[key]);
        } else {
            // Create new vertex
            mesh.vertices.push_back({positions[v_idx], normal, uv});
            uint32_t new_index = static_cast<uint32_t>(mesh.vertices.size() - 1);
            unique_vertices[key] = new_index;
            mesh.indices.push_back(new_index);
        }
        i++;
    }
    return true;
}


std::optional<MeshData> ObjLoader::load(std::string file_path) {
	MeshData mesh;

    std::ifstream file;
    file.open(file_path);

    if (!file.is_open()) {
        std::cerr << "<voxeng> error : obj file " << file_path << " not found.\n";
        return std::nullopt;
    }

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vec3> uvs;
    std::unordered_map<ObjVertexIndex, uint32_t> unique_vertices;

    std::string line;
    bool ok = true;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream line_stream(line);
        std::string token;
        line_stream >> token;

        if (token == "v") {
            parseVec3(line_stream, positions);
        } else if (token == "vt") {
            parseVec3(line_stream, uvs);
        } else if (token == "vn") {
            parseVec3(line_stream, normals);
        } else if (token == "f") {
            if (!processFace(line_stream, positions, normals, uvs, unique_vertices, mesh)) {
                ok = false;
            }
        } else {
            std::cerr << "Unsupported/Unrecognized obj token " << token << "\n";
            ok = false;
        }

        if (!ok) break;
    }

    file.close();
    return ok ? std::make_optional(mesh) : std::nullopt;
}
