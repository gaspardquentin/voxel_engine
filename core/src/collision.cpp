#include "voxel_engine/collision.h"
#include <cmath>

namespace voxeng::collision {

bool isColliding(const WorldCoord &pos, const Vec3f &size, const std::function<bool (Vec3i)>& isSolid) {
    int x_min = (int)std::floor(pos.x); int x_max = (int)std::floor(pos.x + size.x - EPSILON);
    int y_min = (int)std::floor(pos.y); int y_max = (int)std::floor(pos.y + size.y - EPSILON);
    int z_min = (int)std::floor(pos.z); int z_max = (int)std::floor(pos.z + size.z - EPSILON);

    for (int x = x_min; x <= x_max; x++) {
        for (int y = y_min; y <= y_max; y++) {
            for (int z = z_min; z <= z_max; z++) {
                if (isSolid(Vec3i(x,y,z))) {
                    return true;
                }
            }
        }
    }
    return false;
}

}
