#pragma once

#include "world.h"
#include <memory>

#define DEFAULT_SAVE_DIR ".saves/"

namespace  voxeng {

using SaveID = size_t;

class SaveManager {
    class Impl;
    std::unique_ptr<Impl> m_impl;
public:
    SaveManager();
    ~SaveManager();
    SaveManager(std::string save_directory);
    std::string saveWorld(const World& world);
    World loadWorld(SaveID save);

    void setSaveDirectory(std::string save_directory);
    std::string getSaveDirectory() const;

    SaveID getLastSave();

};

}
