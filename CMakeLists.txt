# VoxelEngine - Minimal Voxel Engine Library
cmake_minimum_required(VERSION 3.16)
project(VoxelEngine VERSION 1.0.0 LANGUAGES CXX)

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
#find_package(OpenGL REQUIRED)

# GLEW for OpenGL loading
find_package(GLEW REQUIRED)

# GLFW for window management (examples only)
if(VOXEL_ENGINE_BUILD_EXAMPLES OR VOXEL_ENGINE_BUILD_TESTS)
    find_package(glfw3 REQUIRED)
endif()

# GLM for math
find_package(glm REQUIRED)

# =============================================================================
# LIBRARY SETUP
# =============================================================================

# Source files
set(VOXEL_ENGINE_SOURCES
    src/core/voxel_engine.cpp
    src/core/world.cpp
    src/core/chunk.cpp
    # src/core/math_utils.cpp
    src/rendering/renderer.cpp
    src/rendering/mesh_builder.cpp
    src/rendering/shader.cpp
    src/rendering/camera.cpp
)

# Public headers
set(VOXEL_ENGINE_PUBLIC_HEADERS
    include/voxel_engine/voxel_engine.h
    include/voxel_engine/world.h
    include/voxel_engine/voxel_types.h
    include/voxel_engine/callbacks.h
    include/voxel_engine/math_utils.h
)

# Internal headers
set(VOXEL_ENGINE_INTERNAL_HEADERS
    #src/internal/chunk.h
    src/core/chunk.h
    src/rendering/renderer.h
    src/rendering/mesh_builder.h
    src/rendering/shader.h
    src/rendering/camera.h
    src/config.h
)

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
target_link_libraries(VoxelEngine
    PUBLIC
        GLEW::GLEW
        #OpenGL::GL
        glm::glm
)

# Embed shaders as strings (optional approach)
# You could also load them at runtime
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
target_compile_definitions(VoxelEngine PRIVATE 
    VOXEL_ENGINE_SHADER_DIR="${SHADER_DIR}"
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
    
    # Find or build a simple test framework (or use built-in CTest)
    add_executable(voxel_tests
        tests/test_world.cpp
        tests/test_chunks.cpp
        #tests/test_math.cpp
    )
    
    target_link_libraries(voxel_tests 
        VoxelEngine::VoxelEngine
        glfw  # For OpenGL context in tests
    )
    
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
    EXPORT VoxelEngineTargets
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
install(EXPORT VoxelEngineTargets
    FILE VoxelEngineTargets.cmake
    NAMESPACE VoxelEngine::
    DESTINATION lib/cmake/VoxelEngine
)

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
