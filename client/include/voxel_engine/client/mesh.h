#pragma once

namespace voxeng::client {

class Mesh {
public:
    virtual ~Mesh() = default;
    virtual void draw() const = 0;
};

} // namespace voxeng::client
