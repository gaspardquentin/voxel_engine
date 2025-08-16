#pragma once

#include "world.h"

enum Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class VoxelEngine {
public:
    VoxelEngine(unsigned int screen_width, unsigned int screen_height);
    ~VoxelEngine();

    World& getWorld();
    void render();
    void processMovementPlayer(Movement mov, float delta_time);
    void processMovementCamera(float xoffset, float yoffset, bool constrain_pitch = true);
    void processZoomCamera(float yoffset);

private:
    class Impl;
    Impl* m_impl;
};
