# Voxel Engine

This project is a voxel engine made from Scratch for fun and learning purposes mainly.
This engine is designed as a library, in order to be integrated into any C++ game project.
It has the goal to be very extensible/modifiable. (Yet as the current state of development it is not the case)

## Installation

```bash
git clone https://github.com/gaspardquentin/voxel_engine.git
cd voxel_engine
cmake -B build
cmake --build build
```

The example application can then be run with:
```bash
./build/examples/simple_demo
```


### Dependencies

Make sure you have the following installed on your system:
- A C++17 compatible compiler
- CMake (3.16+)
- OpenGL for rendering
- GLEW
- GLFW for windowing and user inputs
- GLM for math operations
- zlib for world save compression

Dear ImGui (for the GUI layer) and stb_image (for texture loading) are fetched automatically by CMake.

Textures from the [Kenny Voxel Pack](https://kenney.nl/assets/voxel-pack) are included in the repository.

## State of the project

This project is far from being released yet.
It is made in C++17 with CMake as the build system.

### Content implemented

- "Home-made" Wavefront .obj file parser/loader (simple, does not support quads yet)
- "Home-made" world save system (with custom binary format)
- Simple Voxel Engine : Chunks, worlds
- Dynamic chunk-generation
- client/server architecture (for future online mode, but also better project architecture)
- simple GUI using Dear ImGUI
- Chat and command system (with some commands like /fill to place multiple voxels at once)

### What will/may be implemented next (ordered by priority - descending)

- Basic perlin noise terrain generation
- Collision and physics system
- Sound system
- Callback system for easier customization of the engine's behavior
- online multiplayer support
- other rendering backends (only if I've the time) such as Vulkan, WebGL or even a custom software renderer


### Project architecture

The project is split in three "modules" : core, client and server

- core : the common data structures used in both the client and the server and the request/event system
- client : the renderer, the world "view", user input handling and the GUI
- server : the "authoritative" world with chunk generation etc..., the entities and game logic, the chat and commands system, the persistence system

The client and the server communicate via a request/event system with queues.
The system is defined in core.

For now the client and server communicates only inside the same process with no real network communication, but should be ready to it once the network layer is properly implemented.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

2025-2026, Gaspard QUENTIN.
