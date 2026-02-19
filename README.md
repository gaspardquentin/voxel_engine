# Voxel Engine

This project is a voxel engine made from Scratch for fun mainly.
This engine is designed as a library, in order to be integrated into any C++ game project.
It has the goal to be very extensible/modifiable. (Yet as the current state of development it is not the case)


## State of the project

This project is far from being released yet.

### Content implemented

- "Home-made" Wavefront .obj file parser/loader (simple, does not support quads yet)
- "Home-made" world save system (with custom binary format)
- Simple Voxel Engine : Chunks, worlds
- Dynamic chunk-generation

### What will be implemented soon (ordered by priority - descending)

- client/server architecture (for future online mode, but also better project architecture)
- Basic perlin noise terrain generation
- Collision system
- Callback system for easier customization of the engine's behavior

### Libraries used

- Dear ImGUI: for GUI layer
- GLFW: for Window, user inputs...
- OpenGL for Rendering (other backends will hopefully be implemented later)
- stb_image: for texture's image loading


## Specifications

This project is made using C++, OpenGL with GLEW and GLFW, std_image library for textures.

In the future, we would like to add other rendering methods, such as a custom software renderer and maybe Vulkan and WebGL

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Author

2025-2026, Gaspard QUENTIN.
