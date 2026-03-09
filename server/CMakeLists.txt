# =============================================================================
# VoxelEngine Server - Authoritative world, persistence, commands
# =============================================================================

file(GLOB_RECURSE SERVER_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SERVER_HEADERS CONFIGURE_DEPENDS "include/*.h" "src/*.h")

add_library(voxel_server STATIC ${SERVER_SOURCES} ${SERVER_HEADERS})
add_library(VoxelEngine::Server ALIAS voxel_server)

target_include_directories(voxel_server
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(voxel_server
    PUBLIC
        VoxelEngine::Core
        ZLIB::ZLIB
)
