# Performance

This file is for benchmarks of the engine.

## Config

- CPU: AMD Ryzen 7 5800X
- GPU: AMD Radeon RX 6700 XT
- Debug build
- Render distance: 16
- `examples/simple` demo, FPS from the debug window

## Async chunk generation (PR #11)

This first benchmark compares one-thread chunk generation (with a queue and a max amount of chunks to generate per tick) with asynchronous chunk generation.

The asynchronous generation made the client slow: it was receiving too many chunks at once and building all their meshes in the same frame. To fix that, the mesh building is now more "budget friendly": it builds by priority order and does a maximum amount of work per frame. Frustum culling was also added.

The test:

Standing still = waited ~30s for the world to load, then not moving.
The 30s wait is because the one-thread version takes about this time to load every chunk at first
(the async version is way faster!!).

The results:

| Version | Standing still | Sprinting |
|---|---|---|
| Before (`eee2d2d`) | ~500 fps | ~700 fps, chunks super slow to load |
| Async generation only (`f73259e`) | ~800 fps | lots of lag |
| Async + mesh budget + frustum culling (`1db0231`) | ~1400 fps | ~1300 fps, no lag |
