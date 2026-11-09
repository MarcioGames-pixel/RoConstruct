# 2008-06 WebAssembly port

This directory is the browser-port boundary for the recovered 2008-06 client.
The native project is **not currently a complete Emscripten target**.

## Browser host prototype

`browser_host.cpp` is a deliberately small Emscripten/WebGL 2 host smoke test.
It creates the browser canvas context, runs a 60 Hz render loop, and receives
basic keyboard and pointer events. It proves that the CI toolchain can produce
and launch a browser WebAssembly module.

**It is not the Roblox client:** it does not load RBX places, instantiate the
2008 engine, draw 3D geometry, run Lua, connect to a server, or provide gameplay.
Do not label its output as a playable client.

## Current native source state

The recovered source tree is maintained in `workspace/rebuild/2008-06`.
Its build recipes target the historical Windows/MSVC environment and link
Win32-era dependencies. Integration of `CRobloxApp`, `CMainFrame`,
`CRobloxView` and `CRobloxDoc` remains incomplete; some link-closure
functions are still stubs.

## Porting sequence

1. Run `python3 wasm/audit_portability.py --source . --report wasm-portability-report.json`
   from the repository root to inventory platform-specific source dependencies.
2. Define platform interfaces for window/event loop, graphics, input, sockets,
   filesystem/content, threads and audio.
3. Implement browser backends with Emscripten APIs and WebGL; keep the native
   Windows backend intact.
4. Replace MSVC-only build flags and port the exact dependency closure
   (including legacy Boost, RakNet, G3D, SDL and image libraries).
5. Add a real build target for the complete client and link recovered source
   without silently omitting modules or replacing behavior with stubs.
6. Verify in a browser: load a place, create the world/player, render frames,
   accept keyboard/mouse input and exercise network/content paths.

## Acceptance criteria

A full-client WASM release must include a non-empty `.wasm`, its matching
JavaScript loader and required data files, plus a browser smoke test that
actually starts the client and renders a frame. A host-only smoke test or a
portable utility library does not satisfy this bar.
