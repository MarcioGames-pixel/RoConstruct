#!/usr/bin/env python3
"""Audit the recovered G3D 6.09 RenderDevice contract for the WebAssembly port.

This is an inventory tool, not a renderer implementation or a build test.
It intentionally reads the public, reconciled 2008-06 source view.
"""
from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path

FILES = {
    "header": "src/Client/Rendering/g3d/include/GLG3D/RenderDevice.h",
    "implementation": "src/Client/Rendering/g3d/GLG3Dcpp/RenderDevice.cpp",
    "scene": "src/Client/Rendering/RenderLib/RenderScene.cpp",
    "mesh": "src/Client/Rendering/RenderLib/Mesh.cpp",
    "material": "src/Client/Rendering/RenderLib/Material.cpp",
    "adorn": "src/Client/Rendering/AppDraw/AdornG3D.cpp",
    "view": "src/Client/RbxView/View.cpp",
}

FIXED_FUNCTION = {
    "glBegin", "glEnd", "glVertex2fv", "glVertex3fv", "glVertex4fv",
    "glNormal3d", "glNormal3fv", "glColor4d", "glTexCoord",
    "glMultiTexCoord", "glMatrixMode", "glLoadIdentity", "glLoadMatrix",
    "glLoadMatrixf", "glMultMatrixf", "glPushMatrix", "glPopMatrix",
    "glShadeModel", "glAlphaFunc", "glPolygonMode", "glTexEnvf",
    "glTexEnvi", "glTexGeni", "glTexGenfv", "glPushClientAttrib",
    "glPopClientAttrib",
}
EXTENSION_SUFFIXES = ("ARB", "EXT", "ATI", "NV")


def read_sources(root: Path) -> dict[str, str]:
    result = {}
    missing = []
    for name, relative in FILES.items():
        path = root / relative
        if path.is_file():
            result[name] = path.read_text(encoding="utf-8", errors="replace")
        else:
            missing.append(str(path))
    if missing:
        raise SystemExit("Missing required source files:\n  " + "\n  ".join(missing))
    return result


def class_body(header: str) -> str:
    match = re.search(r"\bclass\s+RenderDevice\s*\{", header)
    if not match:
        raise SystemExit("Could not locate G3D::RenderDevice class declaration")
    start = match.end()
    depth = 1
    for index in range(start, len(header)):
        if header[index] == "{":
            depth += 1
        elif header[index] == "}":
            depth -= 1
            if depth == 0:
                return header[start:index]
    raise SystemExit("Unterminated RenderDevice class declaration")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True,
                        help="root of the reconciled 2008-06 source view")
    parser.add_argument("--json", type=Path, required=True)
    parser.add_argument("--markdown", type=Path, required=True)
    args = parser.parse_args()

    sources = read_sources(args.source)
    body = class_body(sources["header"])
    gl_calls = Counter(re.findall(r"\b(gl[A-Z]\w*)\s*\(", sources["implementation"]))
    rd_calls = Counter()
    for name, source in sources.items():
        if name == "header" or name == "implementation":
            continue
        rd_calls.update(re.findall(r"(?:\brd|\brenderDevice)\s*->\s*(\w+)\s*\(", source))

    fixed = sorted(name for name in gl_calls if name in FIXED_FUNCTION)
    extensions = sorted(name for name in gl_calls
                        if name.endswith(EXTENSION_SUFFIXES))
    webgl_risks = sorted(name for name in gl_calls
                         if name in {"glDrawBuffer", "glReadBuffer", "glPolygonMode",
                                     "glAlphaFunc", "glBegin", "glEnd",
                                     "glPushClientAttrib", "glPopClientAttrib",
                                     "glTexGeni", "glTexGenfv"})

    report = {
        "scope": "G3D 6.09 RenderDevice and recovered Roblox RenderLib call sites",
        "source_root": str(args.source),
        "files": FILES,
        "renderdevice_is_polymorphic": bool(re.search(r"\bvirtual\b", body)),
        "renderdevice_virtual_keyword_count": len(re.findall(r"\bvirtual\b", body)),
        "renderdevice_gl_calls": {
            name: count for name, count in sorted(gl_calls.items())
        },
        "fixed_function_calls": fixed,
        "extension_calls": extensions,
        "webgl2_review_required": webgl_risks,
        "renderlib_renderdevice_calls": {
            name: count for name, count in sorted(rd_calls.items())
        },
        "counts": {
            "unique_gl_functions": len(gl_calls),
            "gl_call_sites": sum(gl_calls.values()),
            "unique_renderdevice_calls": len(rd_calls),
            "renderdevice_call_sites": sum(rd_calls.values()),
        },
        "limitations": [
            "Inventory only; it does not compile or execute the renderer.",
            "The recovered RenderDevice class has no virtual methods in its class body.",
            "A derived WebGL2 device cannot replace calls made through G3D::RenderDevice*.",
            "The implementation contains desktop fixed-function and extension GL calls.",
            "A correct port needs a controlled backend seam or a source-level RenderDevice refactor.",
        ],
    }

    args.json.parent.mkdir(parents=True, exist_ok=True)
    args.markdown.parent.mkdir(parents=True, exist_ok=True)
    args.json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    lines = [
        "# G3D 6.09 RenderDevice → WebGL 2 portability audit",
        "",
        "> Generated inventory, not a renderer build or gameplay test.",
        "",
        "## Findings",
        "",
        f"- Unique OpenGL functions in RenderDevice.cpp: **{report['counts']['unique_gl_functions']}**",
        f"- OpenGL call sites in RenderDevice.cpp: **{report['counts']['gl_call_sites']}**",
        f"- Unique RenderDevice methods called by recovered client rendering code: **{report['counts']['unique_renderdevice_calls']}**",
        f"- RenderDevice call sites in those files: **{report['counts']['renderdevice_call_sites']}**",
        f"- Virtual methods declared inside RenderDevice: **{report['renderdevice_virtual_keyword_count']}**",
        "",
        "The recovered RenderDevice is non-polymorphic. A subclass cannot transparently replace",
        "the concrete G3D::RenderDevice methods used by RenderScene and RbxView.",
        "",
        "## Fixed-function calls requiring replacement",
        "",
    ]
    lines.extend(f"- `{name}` ({gl_calls[name]} call sites)" for name in fixed)
    lines += ["", "## Extension calls requiring WebGL capability review", ""]
    lines.extend(f"- `{name}` ({gl_calls[name]} call sites)" for name in extensions)
    lines += ["", "## RenderLib / AppDraw RenderDevice call contract", "",
              "| Method | Call sites |", "|---|---:|"]
    lines.extend(f"| `{name}` | {count} |" for name, count in sorted(rd_calls.items()))
    lines += [
        "",
        "## Required architecture",
        "",
        "1. Add an explicit backend seam inside G3D::RenderDevice (or refactor it behind",
        "   a stable interface); do not rely on subclassing the current concrete class.",
        "2. Keep the native desktop OpenGL implementation as the default backend.",
        "3. Implement the WebGL 2 backend using shader-based transforms, vertex/index buffers,",
        "   texture objects, framebuffer objects, and explicit render-state tracking.",
        "4. Convert legacy immediate-mode and fixed-function operations before wiring",
        "   RenderScene::render() into the browser frame loop.",
        "5. Gate unsupported features (stencil shadow volumes, cube-map reflection,",
        "   polygon mode, texture generation) until WebGL equivalents are verified.",
        "",
        "## Limitations",
        "",
    ]
    lines.extend(f"- {item}" for item in report["limitations"])
    args.markdown.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
