# Repository Guidelines

## Project Structure & Module Organization

This repository contains a small Win32 C++ game and engine prototype. Core source files live at the repository root:

- `DungeonMiner.cpp` / `DungeonMiner.h`: application entry points, game lifecycle hooks, and gameplay glue.
- `GameEngine.*`, `Sprite.*`, `Bitmap.*`, `Background.*`: reusable engine primitives.
- `Player.*`, `ProceduralMapGeneration.*`, `LightMask.*`, `Inventory.h`: dungeon gameplay systems.
- `bitmaps/`: runtime bitmap assets, including ore, tileset, stairs, and player animation frames.
- `DungeonMiner.rc`, `Resource.h`, `DungeonMiner.ico`, `DungeonMiner_sm.ico`: Windows resources.

The Visual Studio solution is `DungeonMiner.sln`; the active project file is `DungeonMiner.vcxproj`.

## Build, Test, and Development Commands

Use a Visual Studio Developer PowerShell or Developer Command Prompt with the MSVC toolchain available.

- `msbuild "DungeonMiner.sln" /p:Configuration=Debug /p:Platform=Win32` builds the debug executable into `Debug/`.
- `msbuild "DungeonMiner.sln" /p:Configuration=Release /p:Platform=Win32` builds the release executable into `Release/`.
- `devenv "DungeonMiner.sln"` opens the project in Visual Studio for debugging and resource inspection.

There is no package manager setup step. Generated build outputs such as `.obj`, `.lib`, `.dll`, and `.exe` should remain untracked.

## Coding Style & Naming Conventions

Follow the existing C++ style: two-space indentation, braces on their own line for functions/classes, and section divider comments for major groups. Keep Win32 naming conventions already in use, such as `m_` member fields, `p` pointer prefixes, `BOOL`, `TCHAR`, `RECT`, and `POINT`. Match existing file pairing with `Feature.cpp` and `Feature.h` when adding new systems.

## Testing Guidelines

No automated tests are currently present. Validate changes by building `Debug|Win32` and manually exercising affected gameplay in the running executable. For deterministic logic such as map generation or inventory behavior, prefer extracting small testable helper functions before adding broad gameplay changes.

## Debug System Notes

`DebugSystem` is configurable at startup. Use `ROIDS_DEBUGSYSTEM=0` to keep it disabled, or `ROIDS_DEBUGSYSTEM=1` to force it on. When disabled, it should stay out of the hot path and not create log files or install exception hooks.

When enabled, debug output is written under `DebugLogs/` with a `session_YYYYMMDD_HHMMSS.log` name. Use `DebugSetPhase()` and `DebugLogFormat()` for lightweight breadcrumbs around gameplay, collision, pathfinding, and exception handling.

## Commit & Pull Request Guidelines

NEVER COMMIT YOURSELF. ONLY USER CAN COMMIT.

## Agent-Specific Instructions

Avoid broad rewrites of the legacy Win32 engine unless requested. Preserve bitmap filenames and project-file entries when moving or adding assets, because Visual Studio resource and project references are path-sensitive. Never create commits in this repository; only the user may commit changes.
