# Rendering System Refactoring Plan

## Objective
Decouple `ChunkRenderer` from `RenderPipeline` to support generic rendering for both World (3D Chunks) and UI (2D Elements) using a unified `GLMesh` and distinct Render Passes.

## Phase 1: Generic Infrastructure (`GLMesh`)
- [x] **Create `src/rendering/opengl/gl_mesh.h` (and `.cpp` if needed)**
    - Extract specific OpenGL buffer logic (VAO, VBO, EBO) from `ChunkRenderer`.
    - **Members**: `VAO`, `VBO`, `EBO`, `index_count`.
    - **Methods**: `upload(MeshData)`, `draw()`, `destroy()`.
- [-] **Refactor `ChunkRenderer` out**
    - Replace usages of `ChunkRenderer` with `GLMesh` in the current codebase to ensure the generic class works.

## Phase 2: Mesh Builders (Data Generation)
- [x] **Analyze `src/rendering/mesh_builder.h`**
    - This is currently specific to Chunks. We will keep it as the "World Builder".
- [x] **Create `src/rendering/interface/ui_mesh_builder.h`**
    - **Responsibility**: Generate `MeshData` (vertices/indices) for UI elements.
    - **Methods**: 
        - `MeshData buildCrosshair()`
        - (Future) `MeshData buildHotbar()`, `MeshData buildText()`.

## Phase 3: Render Passes (Frame Orchestration)
- [ ] **Refine `src/rendering/opengl/opengl_render_pass.h`**
    - Ensure it supports a `render(World&, Camera&)` interface (or similar context).
- [ ] **Create `src/rendering/passes/chunk_render_pass.h`**
    - **Inherits**: `OpenGLRenderPass`.
    - **State**: Enables `GL_DEPTH_TEST`, sets Perspective Projection.
    - **Logic**: Iterates over World Chunks, checks for updates, uses `MeshBuilder` to get data, uploads to `GLMesh`, calls `draw()`.
- [ ] **Create `src/rendering/passes/ui_render_pass.h`**
    - **Inherits**: `OpenGLRenderPass`.
    - **State**: Disables `GL_DEPTH_TEST` (or handles layering), sets Orthographic Projection.
    - **Logic**: Owns `GLMesh` instances for UI elements (crosshair, etc.), uses `UIMeshBuilder` to get data, calls `draw()`.

## Phase 4: Pipeline Integration
- [ ] **Refactor `RenderPipeline` (`src/rendering/renderer.h`)**
    - Remove hardcoded crosshair generation and drawing code.
    - Remove direct Chunk loop from the main render function.
    - **New Member**: `std::vector<std::unique_ptr<OpenGLRenderPass>> m_passes`.
    - **New Logic**: 
        1. Clear Screen.
        2. Iterate through `m_passes`.
        3. Call `pass->render(...)`.
