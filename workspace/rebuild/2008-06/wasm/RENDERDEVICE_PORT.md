# G3D RenderDevice WebAssembly port notes

The recovered client uses the concrete G3D 6.09 `G3D::RenderDevice` class.
`RbxView::View::render(void*)` casts the supplied pointer to
`G3D::RenderDevice*` and calls `RenderScene::render()`. The scene renderer,
mesh uploader and adorn code call that concrete API directly.

The public class declaration contains no virtual methods. A subclass such as
`WebGL2RenderDevice : G3D::RenderDevice` would not override the methods invoked
through the existing `G3D::RenderDevice*` pointers. Merely creating a separate
WebGL class therefore cannot connect the recovered scene renderer.

The G3D implementation also contains legacy desktop OpenGL operations,
including immediate-mode geometry, matrix stack, fixed-function lighting,
alpha test, texture environment/generation, client attribute stack, and
ARB/EXT/ATI extension calls. WebGL 2 requires shader-based replacements and
explicit state/buffer management for these operations.

## Port sequence

1. Use the generated `renderdevice-portability.md` report to track the exact
   RenderDevice method contract and OpenGL operations.
2. Introduce a controlled backend seam inside G3D::RenderDevice, preserving
   the native OpenGL path unchanged by default.
3. Implement WebGL 2 equivalents for frame/state management, transforms,
   indexed geometry, textures and framebuffer operations.
4. Convert or gate legacy features that have no direct WebGL 2 equivalent.
5. Only then connect the existing RenderScene and RbxView frame lifecycle.

The existing `webgl2_device.*` files remain a standalone WebGL 2 smoke-test
backend until that seam exists. They are not a G3D RenderDevice replacement.
