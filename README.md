![WORKMANs](sandbox/assets/textures/WORKMANs_icon_69.png)

# WORKMANs

A DCC application written from scratch: a Vulkan-rendered 3D editor built on an
ECS scene, aimed at covering modeling through animation in one lightweight tool.

The motivation is rig performance. In Maya and Blender the viewport frame rate
collapses once a character rig gets complex, which makes it hard to judge a rig
while animating it. The goal here is a data layout that stays interactive at that
scale rather than one that degrades with it.

**Status: in development.** The editor runs — viewport, camera, gizmo, hierarchy,
selection, mouse picking and scene save/load all work. Modeling and animation
tools are not there yet.

## Implemented

**Renderer (Vulkan)**
- Instance / device / swapchain, plus an offscreen render target drawn into an
  ImGui viewport panel
- The graphics pipeline is a first-class object rather than something hidden
  inside a shader, so draws can later be sorted by pipeline to cut state changes
- Each render target owns its render pass. The viewport target is
  `RGBA8 + R32_SINT + Depth`, so the entity id is written alongside the color
- Dedicated transfer-queue path with ownership-transfer barriers, falling back to
  the graphics family on devices without a separate DMA queue

**Scene (EnTT ECS)**
- Components are split by the question they answer: identity, transform,
  hierarchy, shape (mesh / curve / NURBS), shading, camera, script
- Local and world transforms are separate components. The render traversal runs
  `view<WorldTransformComponent, MeshComponent>` and never touches the local
  data, so it reads ~76 contiguous bytes per entity instead of reaching into a
  ~230 byte struct to use 64 of them
- World matrices are folded once per frame, root down to descendants
- Dirty propagation by generation compare rather than flags: the reader asks
  "was this built from the version I am holding", so a missed write cannot
  silently leave an object frozen
- Rotation is stored as one of six Euler orders or as a quaternion, selected per
  object (the Blender model), with the conversion pinned to a single function

**Editor**
- Outliner with drag-and-drop parenting, Maya-style selection order and
  whole-subtree delete
- ImGuizmo manipulation in world space; the local transform is back-solved under
  the parent by a single owner so the gizmo and reparenting cannot diverge
- Mouse picking reads the `R32_SINT` attachment. The copy is recorded into the
  staging buffer of the current frame-in-flight slot and harvested two frames
  later, so the readback never stalls the pipeline
- Shortcuts live in `sandbox/assets/config/keymap.yaml`, not in code, so menu
  labels and bindings come from one declarative source
- Scene serialization to `.wmsa` (yaml-cpp)

## Planned

- Deferred destruction queue. Resources are currently destroyed after
  `vkDeviceWaitIdle`, which is fine while deletion is a user action but stops
  being acceptable once modifiers regenerate geometry every frame
- Draw sorting by pipeline
- CPU-side editable `Mesh` and a modifier stack — Blender's UX, ECS storage

## Build (Windows)

Requires the **Vulkan SDK** (with `VULKAN_SDK` set) and **Visual Studio 2022**.
premake5 is included under `vendor/bin/premaker`.

1. Clone with submodules:

   ```
   git clone --recurse-submodules https://github.com/workmans-dev/app_workmans.git
   ```

2. Run `setup_vendor.bat` once.

   GLFW, imgui and yaml-cpp are built as part of this solution, so each needs a
   `premake5.lua` — but they are submodules, and a file added inside a submodule
   would have to be committed to someone else's repository. So the originals
   live in `app_workmans/vendor/premaker_files/` and are copied in at setup time.

   The script also registers those copies, the generated `.vcxproj` files and the
   build output in each submodule's `.git/info/exclude`. That file is local to the
   clone and is not tracked, so the submodules stay clean and `git pull` inside
   them never conflicts. (Putting the same rules in a submodule's `.gitignore`
   would do the opposite — that file *is* tracked upstream.)

3. Run `gen_project_win.bat` to generate `app_workmans.sln`.

4. Build the solution. Shaders are compiled to SPIR-V as a prebuild step
   (`compile_shaders.bat`), so editing a shader and rebuilding is enough — there
   is no runtime shader compilation.

Submodules: GLFW, imgui (docking), glm, spdlog, yaml-cpp, ImGuizmo.
Vendored as single headers under `app_workmans/vendor/`: EnTT 3.14.0,
AMD Vulkan Memory Allocator 3.3.0, stb_image.

## Origin

Started by following TheCherno's Hazel engine series on YouTube, then redesigned
the structure and moved the renderer to Vulkan.
