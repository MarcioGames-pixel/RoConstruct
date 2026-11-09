#!/usr/bin/env python3
"""Audit the recovered 2008-06 C++ tree for browser/WASM porting blockers.

This is a source inventory, not a compiler and not proof that the client runs.
It intentionally reports platform dependencies instead of replacing APIs blindly.
"""
from __future__ import annotations

import argparse
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

SOURCE_EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
RULES = {
    "win32_headers": re.compile(r"^\s*#\s*include\s*[<\"](?:windows|windef|winbase|winuser|wingdi|winsock2?|ws2tcpip|commctrl|shellapi|shlobj|wininet|winhttp|winreg|process|io|direct|d3d[^/\\]*)\.h[>\"]", re.I | re.M),
    "mfc_headers": re.compile(r"^\s*#\s*include\s*[<\"](?:afx[^>\"]*|atl[^>\"]*)[>\"]", re.I | re.M),
    "graphics_legacy": re.compile(r"^\s*#\s*include\s*[<\"](?:d3d[^>\"]*|ddraw|dinput|dsound|gl/[^>\"]*)[>\"]", re.I | re.M),
    "windows_symbols": re.compile(r"\b(?:HWND|HINSTANCE|HDC|HANDLE|DWORD|HRESULT|WPARAM|LPARAM|CRITICAL_SECTION|CreateWindowEx|DispatchMessage|GetMessage|PeekMessage|LoadLibrary|GetProcAddress|WSAStartup|closesocket|CreateFile|VirtualAlloc|CoInitialize|IDirect3D\w*)\b"),
    "compiler_extensions": re.compile(r"\b__declspec\s*\(|\b__forceinline\b|\b__try\b|\b__except\b|#\s*pragma\s+comment\s*\("),
}
EXCLUDED_PARTS = {".git", "out", "build", "work", "vendor-cache"}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--report", type=Path, default=Path("wasm-portability-report.json"))
    args = parser.parse_args()
    root = args.source.resolve()
    counts: Counter[str] = Counter()
    examples: dict[str, list[dict[str, str]]] = defaultdict(list)
    scanned = 0

    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in SOURCE_EXTENSIONS:
            continue
        relative = path.relative_to(root)
        if any(part in EXCLUDED_PARTS for part in relative.parts):
            continue
        try:
            content = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        scanned += 1
        for category, pattern in RULES.items():
            matches = list(pattern.finditer(content))
            if not matches:
                continue
            counts[category] += len(matches)
            if len(examples[category]) < 30:
                for match in matches[: 30 - len(examples[category])]:
                    line = content.count("\n", 0, match.start()) + 1
                    snippet = content[match.start():content.find("\n", match.start())]
                    examples[category].append({
                        "file": relative.as_posix(),
                        "line": line,
                        "match": snippet.strip()[:180],
                    })

    report = {
        "source_root": str(root),
        "scanned_files": scanned,
        "classification": "inventory_only_not_a_wasm_build_or_runtime_test",
        "blocker_counts": dict(sorted(counts.items())),
        "examples": dict(examples),
        "next_steps": [
            "Port the WindowsClient entry point and message loop to an Emscripten browser main loop.",
            "Replace Win32, Winsock, MFC/ATL and COM usage with explicit platform interfaces.",
            "Port the renderer from legacy Direct3D/desktop OpenGL assumptions to WebGL 1/2.",
            "Provide browser-safe networking, filesystem/content loading, audio and input backends.",
            "Replace MSVC-only flags and compile the exact dependency closure with an Emscripten toolchain.",
            "Complete CRobloxApp, CMainFrame, CRobloxView and CRobloxDoc integration and remove link stubs.",
            "Add a real CMake target, then validate loading a place, rendering a frame and processing input in a browser."
        ],
    }
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({
        "scanned_files": scanned,
        "blocker_counts": report["blocker_counts"],
        "report": str(args.report),
    }, indent=2))
    return 0 if scanned else 2


if __name__ == "__main__":
    raise SystemExit(main())
