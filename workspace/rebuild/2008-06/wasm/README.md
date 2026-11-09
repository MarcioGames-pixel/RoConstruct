# 2008-06 WebAssembly port

This directory is the browser-port boundary for the recovered 2008-06 client.
The native project is **not currently a complete Emscripten target**.

## Browser graphics host prototype

`browser_host.cpp` owns the Emscripten context, browser events and 60 Hz loop.
The new `webgl2_device.h/.cpp` boundary owns WebGL 2 shader, program, VAO,
vertex-buffer and frame operations. It currently draws diagnostic geometry
(a pulsing colored triangle) and is compiled as part of the browser host.
The canvas backing store is 960x540.

This is still a graphics-pipeline smoke test, **not the historical Roblox renderer**.
`WebGL2Device` is not a subclass or drop-in replacement for `G3D::RenderDevice`.
It does not yet link G3D 6.09, RenderLib, AppDraw, or the recovered RbxView code.
It does not load RBX places, instantiate the 2008 engine, run Lua, connect to
a game server, or provide gameplay. Do not label its output as a playable client.

## G3D RenderDevice port contract

The actual G3D 6.09 API and its Roblox RenderLib call sites have now been
audited. See [RENDERDEVICE_PORT.md](RENDERDEVICE_PORT.md) and the
`renderdevice_portability.py` report generator. The recovered
`G3D::RenderDevice` has no virtual methods, so a derived WebGL class cannot
replace the concrete pointer used by `RbxView` and `RenderScene`. The next
real integration step must add a backend seam inside RenderDevice (or refactor
it behind a stable interface) while preserving the native OpenGL path.

The workflow generates `renderdevice-portability-report.json` and
`renderdevice-portability-report.md` to track the exact legacy GL calls and
RenderLib method contract.

## Current native source state

The recovered source tree is maintained in `workspace/rebuild/2008-06`.
Its build recipes target the historical Windows/MSVC environment and link
Win32-era dependencies. The module manifest for RbxgsApp still expects the
legacy G3D/G3D-GL, Boost, libpng and RakNet dependency closure. Integration of
`CRobloxApp`, `CMainFrame`, `CRobloxView` and `CRobloxDoc` remains
incomplete; some link-closure functions are still stubs.

## Next renderer integration work

1. Map the exact G3D 6.09 `RenderDevice` public contract and its GLG3D call sites
   before attempting a subclass/compatibility implementation.
2. Extend `WebGL2Device` with the required shader, texture, buffer, framebuffer,
   depth, blend and render-state operations; keep unsupported legacy GL behavior
   explicit rather than silently emulating it incorrectly.
3. Adapt RenderLib and AppDraw to that browser backend, then connect RbxView's
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
