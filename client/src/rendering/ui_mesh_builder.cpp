#include "rendering/ui_mesh_builder.h"
#include "rendering/mesh_data.h"

namespace voxeng::client {

MeshData UIMeshBuilder::buildCrosshair() const {
    return crosshair;
}

} // namespace voxeng::client
