# Repository Guidelines

## Project Structure & Module Organization

This repository contains a small Win32 C++ game and engine prototype. Core source files live at the repository root:

- `Roids.cpp` / `Roids.h`: application entry points, game lifecycle hooks, and gameplay glue.
- `GameEngine.*`, `Sprite.*`, `Bitmap.*`, `Background.*`: reusable engine primitives.
- `Player.*`, `ProceduralMapGeneration.*`, `LightMask.*`, `Inventory.h`: dungeon gameplay systems.
- `bitmaps/`: runtime bitmap assets, including ore, tileset, stairs, and player animation frames.
- `Roids.rc`, `Resource.h`, `Roids.ico`, `Roids_sm.ico`: Windows resources.

The Visual Studio solution is `Roids 2.sln`; the active project file is `Roids 2.vcxproj`.

## Build, Test, and Development Commands

Use a Visual Studio Developer PowerShell or Developer Command Prompt with the MSVC toolchain available.

- `msbuild "Roids 2.sln" /p:Configuration=Debug /p:Platform=Win32` builds the debug executable into `Debug/`.
- `msbuild "Roids 2.sln" /p:Configuration=Release /p:Platform=Win32` builds the release executable into `Release/`.
- `devenv "Roids 2.sln"` opens the project in Visual Studio for debugging and resource inspection.

There is no package manager setup step. Generated build outputs such as `.obj`, `.lib`, `.dll`, and `.exe` should remain untracked.

## Coding Style & Naming Conventions

Follow the existing C++ style: two-space indentation, braces on their own line for functions/classes, and section divider comments for major groups. Keep Win32 naming conventions already in use, such as `m_` member fields, `p` pointer prefixes, `BOOL`, `TCHAR`, `RECT`, and `POINT`. Match existing file pairing with `Feature.cpp` and `Feature.h` when adding new systems.

## Testing Guidelines

No automated tests are currently present. Validate changes by building `Debug|Win32` and manually exercising affected gameplay in the running executable. For deterministic logic such as map generation or inventory behavior, prefer extracting small testable helper functions before adding broad gameplay changes.

## Commit & Pull Request Guidelines

NEVER COMMIT YOURSELF. ONLY USER CAN COMMIT.

## Agent-Specific Instructions

Avoid broad rewrites of the legacy Win32 engine unless requested. Preserve bitmap filenames and project-file entries when moving or adding assets, because Visual Studio resource and project references are path-sensitive. Never create commits in this repository; only the user may commit changes.
