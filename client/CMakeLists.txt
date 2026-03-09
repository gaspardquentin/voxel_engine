# =============================================================================
# VoxelEngine Client - Rendering, input, GUI
# =============================================================================

file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE CLIENT_HEADERS CONFIGURE_DEPENDS "include/*.h" "src/*.h")

add_library(voxel_client STATIC ${CLIENT_SOURCES} ${CLIENT_HEADERS})
add_library(VoxelEngine::Client ALIAS voxel_client)

target_include_directories(voxel_client
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Asset paths
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
target_compile_definitions(voxel_client PUBLIC
    VOXEL_ENGINE_SHADER_DIR="${SHADER_DIR}"
    VOXEL_ENGINE_ASSETS_DIR="${ASSETS_DIR}"
)

if (LINUX)
    find_package(OpenGL REQUIRED)
    set(OPENGL_GL OpenGL::GL)
else()
    set(OPENGL_GL)
endif()

target_link_libraries(voxel_client
    PUBLIC
        VoxelEngine::Core
        VoxelEngine::Server  # temporary: client still depends on World directly
        GLEW::GLEW
        ${OPENGL_GL}
    PRIVATE
        imgui
)
