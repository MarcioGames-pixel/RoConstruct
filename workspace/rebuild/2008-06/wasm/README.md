# 2008-06 WebAssembly port

This directory is the browser-port boundary for the recovered 2008-06 client.
The native project is **not currently a complete Emscripten target**.

## Browser graphics host prototype

`browser_host.cpp` now creates a WebGL 2 context, compiles GLSL ES 3.00
vertex/fragment shaders, uploads an interleaved vertex buffer, configures a
vertex array, and draws a pulsing colored triangle in a 60 Hz browser loop.
It also receives basic keyboard and pointer events. The canvas backing store
is 960x540.

This is a graphics-pipeline smoke test, **not the historical Roblox renderer**.
It does not link G3D 6.09, RenderLib, AppDraw, or the recovered RbxView code.
It does not load RBX places, instantiate the 2008 engine, run Lua, connect to
a game server, or provide gameplay. Do not label its output as a playable client.

## Current native source state

The recovered source tree is maintained in `workspace/rebuild/2008-06`.
Its build recipes target the historical Windows/MSVC environment and link
Win32-era dependencies. The module manifest for RbxgsApp still expects the
legacy G3D/G3D-GL, Boost, libpng and RakNet dependency closure. Integration of
`CRobloxApp`, `CMainFrame`, `CRobloxView` and `CRobloxDoc` remains
incomplete; some link-closure functions are still stubs.

## Next renderer integration work

1. Port the G3D 6.09 GLG3D/RenderDevice layer to a WebGL2-backed device while
   preserving the existing native renderer.
2. Replace desktop OpenGL calls and unsupported state/features with explicit
   WebGL2 equivalents; add shader, texture, buffer and framebuffer adapters.
3. Adapt RenderLib and AppDraw to the browser device, then connect RbxView's
   frame/render lifecycle to the Emscripten main loop.
4. Replace MSVC-only flags and port the exact dependency closure (legacy
   Boost, RakNet, G3D, SDL and image libraries).
5. Complete the application/document/view integration and remove link stubs.
6. Validate loading a real place, constructing the world/player, drawing the
   real scene, accepting input and exercising network/content paths.

## Acceptance criteria

A full-client WASM release must include a non-empty `.wasm`, its matching
JavaScript loader and required data files, plus a browser smoke test that
loads a real place and renders the actual game scene. A host-only triangle or
portable utility library does not satisfy this bar.
