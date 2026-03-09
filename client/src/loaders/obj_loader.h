#pragma once

#include "rendering/mesh_data.h"
#include <optional>

namespace voxeng::client {

struct ObjLoader {
	static std::optional<MeshData> load(std::string file_path);
};

} // namespace voxeng::client
