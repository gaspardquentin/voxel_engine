# VoxelEngine - Minimal Voxel Engine Library
cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# CMAKE POLICIES
# =============================================================================

# Fix the FetchContent timestamp warning
cmake_policy(SET CMP0135 NEW)

# =============================================================================
# PROJECT OPTIONS
# =============================================================================

option(VOXEL_ENGINE_BUILD_EXAMPLES "Build example programs" ON)
option(VOXEL_ENGINE_BUILD_TESTS "Build tests" ON)
option(VOXEL_ENGINE_BUILD_SHARED "Build as shared library" OFF)

# =============================================================================
# COMPILER SETTINGS
# =============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup Google Test early if needed
if(VOXEL_ENGINE_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Debug/Release configurations
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific flags
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3)
    endif()
endif()

# =============================================================================
# DEPENDENCIES
# =============================================================================

# OpenGL

if (LINUX)
    find_package(OpenGL REQUIRED)
endif()

# GLEW for OpenGL loading
find_package(GLEW REQUIRED)

# GLFW for window management (examples only)
if(VOXEL_ENGINE_BUILD_EXAMPLES OR VOXEL_ENGINE_BUILD_TESTS)
    find_package(glfw3 REQUIRED)
endif()

# GLM for math
find_package(glm REQUIRED)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# =============================================================================
# LIBRARY SETUP
# =============================================================================

# Source files
file(GLOB_RECURSE VOXEL_ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Public headers
set(VOXEL_ENGINE_PUBLIC_HEADERS
    include/voxel_engine/voxel_engine.h
    include/voxel_engine/world.h
    include/voxel_engine/voxel_types.h
    include/voxel_engine/callbacks.h
    include/voxel_engine/math_utils.h
    include/voxel_engine/texture.h
    include/voxel_engine/chunk.h
)

# Internal headers
file(GLOB_RECURSE VOXEL_ENGINE_INTERNAL_HEADERS CONFIGURE_DEPENDS "src/*.h")

# Create library
if(VOXEL_ENGINE_BUILD_SHARED)
    add_library(VoxelEngine SHARED ${VOXEL_ENGINE_SOURCES} ${VOXEL_ENGINE_INTERNAL_HEADERS})
    target_compile_definitions(VoxelEngine PRIVATE VOXEL_ENGINE_EXPORTS)
    target_compile_definitions(VoxelEngine PUBLIC VOXEL_ENGINE_SHARED)
else()
    add_library(VoxelEngine STATIC ${VOXEL_ENGINE_SOURCES} ${VOXEL_ENGINE_INTERNAL_HEADERS})
endif()

# Create alias for consistent usage
add_library(VoxelEngine::VoxelEngine ALIAS VoxelEngine)

# =============================================================================
# LIBRARY CONFIGURATION
# =============================================================================

# Include directories
target_include_directories(VoxelEngine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link dependencies
if (LINUX)
    set(OPENGL_GL OpenGL::GL)
else()
    set(OPENGL_GL)
endif()

target_link_libraries(VoxelEngine
    PUBLIC
        GLEW::GLEW
        ${OPENGL_GL}
        glm::glm
    PRIVATE
        imgui
)

# Setup ImGui Library
# Core ImGui (No backends, no GLFW dependency)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# ImGui Backends (Depends on GLFW and OpenGL)
if(TARGET glfw)
    add_library(imgui_backends STATIC
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui_backends PUBLIC ${imgui_SOURCE_DIR}/backends)
    target_compile_definitions(imgui_backends PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
    target_link_libraries(imgui_backends PUBLIC imgui glfw ${OPENGL_GL} GLEW::GLEW)
endif()

# Embed shaders as strings (optional approach)
# You could also load them at runtime
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_compile_definitions(VoxelEngine PUBLIC
    VOXEL_ENGINE_SHADER_DIR="${SHADER_DIR}"
    VOXEL_ENGINE_ASSETS_DIR="${ASSETS_DIR}"
)

# =============================================================================
# EXAMPLES
# =============================================================================

if(VOXEL_ENGINE_BUILD_EXAMPLES)
    # Basic Demo
    add_executable(simple_demo
        examples/simple/main.cpp
    )
    target_link_libraries(simple_demo
        VoxelEngine::VoxelEngine
        glfw
        imgui_backends
        #GLEW::GLEW
        #OpenGL::GL
    )

    # Terrain Generation Demo
    #add_executable(terrain_gen
    #examples/terrain_gen/main.cpp
    #)
    #target_link_libraries(terrain_gen
    #VoxelEngine::VoxelEngine
    #glfw
    #)

    # Set output directory for examples
    set_target_properties(simple_demo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endif()

# =============================================================================
# TESTS
# =============================================================================

if(VOXEL_ENGINE_BUILD_TESTS)
    enable_testing()

    # Create test executable
    add_executable(voxel_tests
        tests/test_world.cpp
        tests/test_chunks.cpp
        #tests/test_math.cpp
    )

    target_link_libraries(voxel_tests
        VoxelEngine::VoxelEngine
        GTest::gtest_main
        glfw  # For OpenGL context in tests
    )

    # Use GoogleTest's test discovery
    include(GoogleTest)
    gtest_discover_tests(voxel_tests)  # Fixed: was "hello_test", should be "voxel_tests"

    # Also add as a CTest test
    add_test(NAME VoxelEngineTests COMMAND voxel_tests)

    set_target_properties(voxel_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
endif()

# =============================================================================
# INSTALLATION
# =============================================================================

# Install targets
install(TARGETS VoxelEngine
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install public headers
install(FILES ${VOXEL_ENGINE_PUBLIC_HEADERS}
    DESTINATION include/voxel_engine
)

# Install shaders
install(DIRECTORY shaders/
    DESTINATION share/voxel_engine/shaders
    FILES_MATCHING PATTERN "*.glsl"
)

# Export targets
# install(EXPORT VoxelEngineTargets
#     FILE VoxelEngineTargets.cmake
#     NAMESPACE VoxelEngine::
#     DESTINATION lib/cmake/VoxelEngine
# )

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "VoxelEngineConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VoxelEngineConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/VoxelEngineConfig.cmake"
    INSTALL_DESTINATION lib/cmake/VoxelEngine
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VoxelEngineConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VoxelEngineConfigVersion.cmake"
    DESTINATION lib/cmake/VoxelEngine
)

# =============================================================================
# PACKAGE CREATION
# =============================================================================

set(CPACK_PACKAGE_NAME "VoxelEngine")
set(CPACK_PACKAGE_VENDOR "YourName")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Minimal voxel engine library")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# =============================================================================
# DEVELOPMENT HELPERS
# =============================================================================

# Print configuration summary
message(STATUS "VoxelEngine Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared library: ${VOXEL_ENGINE_BUILD_SHARED}")
message(STATUS "  Build examples: ${VOXEL_ENGINE_BUILD_EXAMPLES}")
message(STATUS "  Build tests: ${VOXEL_ENGINE_BUILD_TESTS}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Clang-format support (if available)
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${VOXEL_ENGINE_SOURCES} ${VOXEL_ENGINE_PUBLIC_HEADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Formatting source code"
    )
endif()
