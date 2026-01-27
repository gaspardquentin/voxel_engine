#pragma once

#include "rendering/mesh_data.h"
#include <optional>

struct ObjLoader {
	static std::optional<MeshData> load(std::string file_path);
};
