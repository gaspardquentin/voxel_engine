#pragma once

#include "world.h"

class VoxelEngine {
public:
    VoxelEngine();
    ~VoxelEngine();

    World& getWorld();
    void render();

private:
    class Impl;
    Impl* m_impl;
};
