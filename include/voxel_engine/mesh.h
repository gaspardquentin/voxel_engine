#pragma once

namespace voxeng {

class Mesh {
public:
    virtual ~Mesh() = default;
    virtual void draw() const = 0;
};

}
