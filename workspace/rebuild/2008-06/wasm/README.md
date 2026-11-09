# 2008-06 WebAssembly port

This directory is the browser-port boundary for the recovered 2008-06 client.
The native project is **not currently a complete Emscripten target**.

## Current state

The canonical source is maintained in `workspace/rebuild/2008-06`. Its
existing build recipes target the historical Windows/MSVC environment and
link Win32-era dependencies. The recovered tree also documents incomplete
integration of `CRobloxApp`, `CMainFrame`, `CRobloxView` and
`CRobloxDoc`; some link-closure functions remain stubs.

A successful compile of isolated utility code would not mean the client is
playable. Do not publish an artifact as a full client until the actual client
entry point, renderer, input, networking, content loading and audio work in a
browser.

## Porting sequence

1. Run `python3 wasm/audit_portability.py --source . --report wasm-portability-report.json`
   from this directory to inventory platform-specific source dependencies.
2. Define platform interfaces for window/event loop, graphics, input, sockets,
   filesystem/content, threads and audio.
3. Implement browser backends with Emscripten APIs and WebGL; keep the native
   Windows backend intact.
4. Replace MSVC-only build flags and port the exact dependency closure
   (including the legacy Boost, RakNet, G3D, SDL and image libraries).
5. Add the real CMake target for the complete client and link the recovered
   source without silently omitting modules or replacing behavior with stubs.
6. Verify in a browser: load a place, create the world/player, render frames,
   accept keyboard/mouse input and exercise network/content paths.

## Acceptance criteria

A full-client WASM release must include a non-empty `.wasm`, its matching
JavaScript loader and required data files, plus a browser smoke test that
actually starts the client and renders a frame. Merely compiling a diagnostic
shell or one portable library does not satisfy this bar.
