# 🧪 VoxelEngine Testing Plan

Hey Gaspard — here's the testing strategy I've put together after digging through your entire codebase.

## Current State

You already have **GoogleTest wired up** in CMake (with `FetchContent`) and three test files — but only [`test_world.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_world.cpp) is active (and it's just a placeholder). The two commented-out files ([`test_chunks.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_chunks.cpp), [`test_world_lookup.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_world_lookup.cpp)) contain real tests written for an older API — they'll need to be updated to match the current architecture.

The codebase has a **clean separation**: `core` (data structures, math, types), `server` (world, generation, persistence, chat, commands, ECS), and `client` (rendering, mesh building). Your network layer uses **abstract interfaces** (`IClientConnection`/`IServerConnection`) with a `LocalTransport` implementation — which is *great* for testing.

---

## Philosophy: What Makes a Test "Smart" Here

Not everything deserves the same level of testing. I've prioritized based on:

1. **Bug likelihood** — Coordinate math, boundary conditions, and serialization are where voxel engines break
2. **Blast radius** — A bug in world generation or chunk loading corrupts *everything*
3. **Testability** — Pure logic with clear inputs/outputs > side-effect-heavy code
4. **Regression value** — Will this test catch real bugs when you refactor?

---

## 🔷 Tier 1 — Unit Tests (Highest Priority)

These test individual functions/classes in isolation. No external dependencies, fast to run.

### 1.1 Math & Coordinate System

**Why it matters:** Coordinate conversion is the *spine* of a voxel engine. A single off-by-one in `getChunkId` or `getChunkPosFromWorld` — especially for negative coordinates — causes chunks to load in wrong positions, voxels to appear in wrong places, etc. This is historically the #1 source of bugs in voxel engines.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **`Vec2T` / `Vec3T`** | Arithmetic ops, `fromString` parsing, `normalize`, `cross`, `dist`, `chebyshev` | Zero vectors, negative values, `fromString` with/without parens, invalid strings throwing `invalid_argument` |
| **`getChunkId(WorldCoord)`** | World position → ChunkID mapping | `(0,0,0)` → `(0,0)`, `(15.9,0,15.9)` → `(0,0)`, `(16.0,0,0)` → `(1,0)`, **`(-0.1,0,0)` → `(-1,0)`**, **`(-16.0,0,0)` → `(-1,0)`**, `(-16.1,0,0)` → `(-2,0)` |
| **`getChunkWorldPos(ChunkID)`** | ChunkID → world position | `(0,0)` → `(0,0,0)`, `(1,0)` → `(16,0,0)`, `(-1,0)` → `(-16,0,0)` |
| **`Chunk::linearIndex`** | 3D coord → flat array index | `(0,0,0)` → 0, `(15,15,15)` → max, verify roundtrip with `getVoxel`/`setVoxel` |
| **`Chunk::positionInChunk`** | Bounds validation | All boundary values: `(0,0,0)` ✓, `(15,15,15)` ✓, `(16,0,0)` ✗, underflow (unsigned wrapping) |
| **`Chunk::getChunkPosFromWorld`** | World pos → local chunk coords | Positive coords, negative coords, exact boundaries |
| **`ChunkID` hashing** | Hash correctness for map usage | Different IDs produce different hashes, symmetric IDs `(1,2)` vs `(2,1)` don't collide |

> [!IMPORTANT]
> The **negative coordinate boundary** is the single most dangerous area. Your existing [`test_world_lookup.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_world_lookup.cpp) already had tests for this — these were clearly written because you hit bugs there. Reviving and expanding them is a top priority.

---

### 1.2 Chunk Data Operations

**Why it matters:** The chunk is the fundamental data container. Its get/set/bounds logic must be bulletproof.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **Set & Get roundtrip** | `setVoxel` then `getVoxel` returns same type | Every voxel type, multiple positions |
| **Out-of-bounds access** | `getVoxel` on invalid coords returns air (type 0) | Beyond width, height, depth |
| **Out-of-bounds set** | `setVoxel` on invalid coords returns 0, doesn't corrupt | Same as above |
| **`setRawData` / `getRawData`** | Bulk data loading works | Load a full array, verify random positions |
| **Dirty flags** | `isRenderDirty` / `isPersistenceDirty` behavior | Set a voxel → dirty, `clear` → not dirty, initial state |

---

### 1.3 World Generation (Chunk Generators)

**Why it matters:** Terrain generation defines the player experience. Bugs here produce floating islands, terrain holes, or wrong block types.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **`FlatGenerator` structural invariants** | Layer composition is correct | Stone below `height-2`, dirt at `height-1`/`height-2`, grass at `height`, air above |
| **`FlatGenerator` consistency** | Same output regardless of ChunkID | Generate at `(0,0)`, `(5,3)`, `(-10,-7)` — all should be identical (flat world) |
| **`PerlinGenerator` determinism** | Same seed → same terrain | Generate chunk `(0,0)` with seed 42 twice, compare `data` arrays |
| **`PerlinGenerator` seed sensitivity** | Different seed → different terrain | Seed 42 vs seed 43 produce different output for same ChunkID |
| **`PerlinGenerator` layer structure** | Stone/dirt/grass layering follows heightmap | For each `(x,z)` column: stone at bottom, dirt in middle, grass on top, air above |
| **`PerlinGenerator` height bounds** | No voxel placed at `y ≥ CHUNK_HEIGHT` | Scan full chunk, verify bounds |
| **Water placement** | Water appears at minimum-height positions | Verify `is_water` logic: water at low-elevation columns, fills up to `min_height` |

> [!TIP]
> For Perlin tests, use **property-based testing** rather than snapshot testing. Don't assert "block at (3,7,2) is stone". Instead assert structural properties: "stone is always below dirt", "grass is always on top of dirt", "no air below ground". This way tests survive generator tuning.

---

### 1.4 Chat System & History

**Why it matters:** The `ChatHistory` uses a **circular buffer** (`m_head`, `m_first`, `m_count`). Ring buffers are a classic source of off-by-one and wraparound bugs.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **Empty history** | `size()` == 0, `getAllMessages()` empty | Initial state |
| **Single message** | Push one, `getLastMessage` and `getFirstMessage` return it | Sender + content preserved |
| **Fill to capacity** | Push exactly `history_length` messages | `size()` == capacity, order preserved |
| **Overflow / wraparound** | Push `history_length + N` messages | Oldest messages evicted, newest preserved, `size()` stays at capacity |
| **`getNLastMessages(n)`** | Correct subset returned | `n=0`, `n=1`, `n=size`, `n > size` |
| **`Chat::sendMessage` empty** | Empty string is rejected (no push) | Verify `size()` doesn't increase |

---

### 1.5 Command Registry & Parsing

**Why it matters:** Command parsing has string manipulation and edge cases. The `/fill` command directly mutates the world — parsing bugs could corrupt it.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **Registration & lookup** | Registered command is found, unregistered returns false | `/known_cmd` → true, `/unknown` → false |
| **Argument parsing** | Input string split correctly by spaces | `/fill air (1,1,1) (10,10,10)` → 3 args |
| **Empty input** | Returns false | `""` |
| **No prefix** | Input without `/` returns false | `"hello"` |
| **`/fill` validation** | Wrong arg count triggers error callback | 0 args, 1 arg, 2 args, 4 args |
| **`/fill` invalid voxel** | Nonexistent type triggers error | `/fill nonexistent (0,0,0) (1,1,1)` |
| **`/fill` coord parsing** | Invalid coords trigger error | `/fill air invalid (1,1,1)` |

---

### 1.6 Entity Component System

**Why it matters:** The movement system is simple but critical — it's the only server-side physics. Wrong delta application means entities teleport or freeze.

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **`movement_system::update`** | Position updated by velocity × dt | Known velocity + known dt → expected position |
| **Zero velocity** | Entity doesn't move | `vel = (0,0,0)`, any dt |
| **Zero delta time** | Entity doesn't move | Any velocity, `dt = 0` |
| **Multiple entities** | Each updated independently | 3 entities with different velocities |

---

### 1.7 Voxel Types

| Test Suite | What to Test | Key Cases |
|---|---|---|
| **`DEFAULT_VOXEL_TYPES` integrity** | Air is id 0, is not solid, is transparent | Verify air properties specifically |
| **Property lookup** | Each type's solid/transparent flags correct | Water is not solid + transparent, stone is solid + not transparent |

---

## 🔶 Tier 2 — Integration Tests (High Priority)

These test multiple components working together, but still no GPU/window/filesystem (or with a temp filesystem).

### 2.1 Server World — Chunk Lifecycle

**Why it matters:** This tests the `World::Impl` orchestration: generation queue, load-from-disk fallback, unloading with persistence, and the event stream. Bugs here = missing chunks or data loss.

**Setup:** Use a `FlatGenerator` (deterministic, no noise dependencies) and a mock `IServerConnection` that captures pushed `ClientEvent`s.

| Test | What to Test |
|---|---|
| **Initial generation** | After construction with `generate_chunks=true`, `getChunks()` is non-empty and events were pushed |
| **`setVoxel` + event** | `setVoxel(pos, id)` updates the chunk AND pushes a `VoxelChangedEvent` to the connection |
| **`getVoxel` consistency** | After `setVoxel`, `getVoxel` returns the new type |
| **Out-of-world `setVoxel`** | Setting a voxel in an unloaded chunk returns 0 and doesn't crash |
| **Player join** | `playerJoin` creates an entity with `PlayerData`, `Position`, `ChunkTracking` components |
| **`updateChunks` — chunk loading** | After moving a player entity, `updateChunks` triggers `_preGenerateChunks` and generates new chunks |
| **`updateChunks` — chunk unloading** | Moving a player far away causes distant chunks to be erased and `ChunkUnloadEvent` pushed |
| **Entity spawn** | `spawnEntity` creates entity with Model+Position components and pushes `EntitySpawnEvent` |

---

### 2.2 Server Request Handling (Full Server Integration)

**Why it matters:** This is the core game loop. The `Server` reads requests from the connection, dispatches to handlers, and pushes events back. Testing this validates the full request→response pipeline **without** any actual networking.

**Setup:** Use `LocalTransport` + `LocalServerConnection` + `LocalClientConnection`. Push requests through the client side, call `server.update()`, poll events from the client side.

| Test | What to Test |
|---|---|
| **Create → Load → Join workflow** | Push `CreateWorldRequest` → verify `NewWorldCreatedEvent`. Push `LoadWorldRequest` → no error. Push `JoinWorldRequest` → receive `WorldLoadEvent` with correct voxel types |
| **Voxel modification** | After joining, push `PlayerVoxelRequest` → receive `VoxelChangedEvent` |
| **Chat message** | Push `SendChatRequest` → receive `ChatMessageEvent` with same content |
| **Chat history on join** | Send messages, then join a new player → they receive `ChatHistoryEvent` |
| **Command execution** | Push `SendChatRequest` with `/fill air (0,0,0) (2,2,2)` → voxels changed |
| **Unknown command** | Push `/nonexistent` → receive `ServerErrorEvent` |
| **Join without world** | Push `JoinWorldRequest` before loading → receive `ServerErrorEvent` |
| **Tick rate gating** | Call `update()` twice quickly — only the first should process requests |

> [!IMPORTANT]
> This is the **highest-value integration test**. It validates the entire server logic without touching the filesystem (except for SaveManager, which can use a temp dir). One test suite here catches more real bugs than 50 unit tests on getters/setters.

---

### 2.3 Persistence Round-Trip (SaveManager)

**Why it matters:** Data loss = losing a player's world. The compressed binary format must be bit-perfect on save→load roundtrips.

**Setup:** Use a temporary directory (in the build tree or `/tmp`) for filesystem operations.

| Test | What to Test |
|---|---|
| **Create + open** | Create a world, close, reopen → metadata matches |
| **Metadata roundtrip** | Write metadata with voxel types + players, read back → identical |
| **Chunk save/load** | Save a `ChunkSaveData` with known content, load → identical byte-for-byte |
| **`chunkExistsOnDisk`** | True after save, false for unsaved ChunkID |
| **`listWorlds`** | Create 3 worlds, list → returns all 3 paths |
| **Format version** | Saved format_version/sub_version match `FORMAT_VERSION`/`FORMAT_SUB_VERSION` |
| **Missing world.meta** | `openWorld` on a dir without meta → returns false |

---

### 2.4 Client/Server Communication via LocalTransport

**Why it matters:** The `LocalTransport` is the production-used communication channel for singleplayer. It's mutex-guarded and shared between threads. Testing it verifies the actual transport mechanism (not just mocked interfaces).

| Test | What to Test |
|---|---|
| **Request roundtrip** | Client pushes request → server polls it → same variant type + data |
| **Event roundtrip** | Server pushes event → client polls it → same variant type + data |
| **FIFO ordering** | Push 3 requests → poll 3 → same order |
| **Empty poll** | Polling with nothing queued → `std::nullopt` |
| **`ChunkDataEvent` roundtrip** | Push a `ChunkDataEvent` with 4096 voxels → poll → verify all data intact |

---

## 🔸 Tier 3 — Functional Tests

These validate higher-level behaviors / user-facing workflows.

### 3.1 World Generation Quality Checks

Beyond structural correctness (Tier 1), verify that the generator produces *reasonable* terrain:

| Test | What to Test |
|---|---|
| **No fully empty chunks** | For seed range `[0..10]`, generate 100 chunks each, verify none are 100% air |
| **No fully solid chunks** | Same — verify none are 100% stone (meaning the surface is always reachable) |
| **Height continuity** | Adjacent chunks shouldn't have massive height discontinuities at shared edges (e.g., > 8 blocks difference at chunk boundaries). This catches visible terrain seams. |
| **Water connectivity** | If water exists in a chunk, it should form a contiguous low-area (not random floating water) |

### 3.2 Command System E2E (via Server)

Test commands from the user's perspective: type a command string → observe world changes.

| Test | What to Test |
|---|---|
| **`/fill` modifies world** | Execute `/fill stone (0,0,0) (3,3,3)` → verify 64 blocks are now stone |
| **`/fill` with reversed coords** | `/fill dirt (3,3,3) (0,0,0)` should work identically (min/max normalization) |
| **`/cow` spawns entity** | Execute `/cow` → verify entity exists in registry with "cow" model |

---

## 🟠 Tier 4 — End-to-End / System Tests

> [!NOTE]
> These are harder to implement and slower to run. I'd recommend tackling them *after* Tiers 1–3 are solid.

### 4.1 Full Singleplayer Session Simulation

A "headless" end-to-end test that simulates the full singleplayer lifecycle without any OpenGL or window:

1. Create `LocalTransport`, `LocalClientConnection`, `LocalServerConnection`
2. Create `Server` with the server-side connection
3. Push `CreateWorldRequest` → `LoadWorldRequest` → `JoinWorldRequest`
4. Call `server.update()` a few times to generate chunks
5. Poll events from client side: verify `WorldLoadEvent` + multiple `ChunkDataEvent`s received
6. Push `PlayerVoxelRequest` to place a block → poll `VoxelChangedEvent`
7. Push `SaveWorldRequest`
8. Destroy everything, create new `Server` + connections
9. Push `LoadWorldRequest` for same world → `JoinWorldRequest`
10. Verify the placed block persists in the loaded chunks

**This single test validates**: transport, server lifecycle, world creation, chunk generation, voxel mutation, persistence, world reloading. It's *the* test that gives you confidence the engine actually works.

### 4.2 Persistence Across Sessions

A variant of 4.1 focused specifically on data durability:

| Test | What to Test |
|---|---|
| **Block persistence** | Place blocks → save → reload → blocks still there |
| **Player data** | Join → save → reload → player in `known_players` |
| **World metadata** | Create with custom voxel types → reload → same types |

---

## 🏗️ Implementation Recommendations

### Test File Organization

```
tests/
├── unit/
│   ├── test_math.cpp              # Vec2T, Vec3T, fromString, etc.
│   ├── test_coordinates.cpp       # getChunkId, getChunkWorldPos, linearIndex
│   ├── test_chunk.cpp             # Chunk data operations
│   ├── test_voxel_types.cpp       # VoxelType properties
│   ├── test_chat_history.cpp      # ChatHistory ring buffer
│   ├── test_command_registry.cpp  # Command parsing & dispatch
│   ├── test_entity_systems.cpp    # Movement system
│   └── test_generators.cpp        # FlatGenerator, PerlinGenerator properties
├── integration/
│   ├── test_server_world.cpp      # World chunk lifecycle
│   ├── test_server_requests.cpp   # Full server request→event pipeline
│   ├── test_local_transport.cpp   # LocalTransport roundtrips
│   └── test_save_manager.cpp      # Persistence roundtrips
├── functional/
│   ├── test_generation_quality.cpp  # Terrain quality assertions
│   └── test_commands_e2e.cpp        # Commands through server
└── e2e/
    └── test_full_session.cpp        # Full singleplayer simulation
```

### Test Helpers to Build

1. **`MockServerConnection`** — Implements `IServerConnection`, captures all pushed `ClientEvent`s into a vector for assertion. Minimal — maybe 20 lines.
2. **`TestWorld` factory** — Helper that creates a `World` with a `FlatGenerator`, mock connection, and `generate_chunks=false` for quick tests that don't need terrain.
3. **Temp directory RAII wrapper** — For `SaveManager` tests. Creates a temp dir in constructor, deletes in destructor.

### CMake Structure

Update [CMakeLists.txt](file:///home/gaspou/documents/code/opengl/voxel_engine/CMakeLists.txt) to:
- Glob or list all test files
- Link against `VoxelEngine::Core` and `VoxelEngine::Server` (no need for `VoxelEngine::Client` or OpenGL for most tests)
- Consider separate test executables per tier if build times matter later

### What NOT to Test (For Now)

| Module | Why Skip |
|---|---|
| **OpenGL rendering** (`gl_mesh.cpp`, render passes) | Requires GPU context. Use rendering-specific testing frameworks (like headless GL contexts) only if visual regression becomes a problem. |
| **GLFW/ImGui integration** | Platform-dependent, hard to automate, low bug density. |
| **`stb_perlin.h`** | Third-party code — trust it, test *your* usage of it. |

---

## 📋 Prioritized Implementation Order

If you want to start implementing, I'd recommend this order:

| # | Suite | Estimated Effort | Impact |
|---|---|---|---|
| 1 | Coordinate math (`getChunkId`, boundaries) | 🟢 Small | 🔴 Critical |
| 2 | Chunk data operations | 🟢 Small | 🔴 Critical |
| 3 | Server request pipeline (integration) | 🟡 Medium | 🔴 Critical |
| 4 | ChatHistory ring buffer | 🟢 Small | 🟡 Medium |
| 5 | Command registry + parsing | 🟢 Small | 🟡 Medium |
| 6 | Generator properties | 🟡 Medium | 🟡 Medium |
| 7 | SaveManager persistence | 🟡 Medium | 🔴 Critical |
| 8 | LocalTransport roundtrips | 🟢 Small | 🟡 Medium |
| 9 | Full session E2E | 🔴 Large | 🔴 Critical |
| 10 | Generation quality | 🟡 Medium | 🟢 Nice-to-have |

---

## Open Questions for You

1. **Existing tests:** The commented-out [`test_chunks.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_chunks.cpp) and [`test_world_lookup.cpp`](file:///home/gaspou/documents/code/opengl/voxel_engine/tests/test_world_lookup.cpp) were written for an older API (`World` constructor taking `(voxel_types, render_dist, seed)`). Should I update these to the current API as a starting point, or write fresh tests from scratch?

2. **CI integration:** Do you have or plan any CI pipeline (GitHub Actions, etc.)? Tests are 10× more valuable when they run automatically on every push.

3. **Test coverage tooling:** Interested in setting up `lcov`/`gcov` coverage reports? Not for chasing coverage numbers, but to spot untested *branches* (e.g., "did we ever test the error path in `SaveManager::loadChunk`?").

Ready to dive into implementation whenever you are, Gaspard! Just tell me which tier/suite to start with.
