# =============================================================================
# VoxelEngine Core - Shared types, data structures, and utilities
# =============================================================================

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE CORE_HEADERS CONFIGURE_DEPENDS "include/*.h" "src/*.h")

add_library(voxel_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
add_library(VoxelEngine::Core ALIAS voxel_core)

target_include_directories(voxel_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
