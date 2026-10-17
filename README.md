# OpenSiv3D Project Template

A Windows x64 OpenSiv3D 0.6.16 project template built with CMake, including
GoogleTest unit tests and spdlog / yaml-cpp support.

## Features

- CMake + Ninja Multi-Config build (Debug and Release)
- OpenSiv3D 0.6.16 application with precompiled header and embedded resources
- GoogleTest unit tests wired into CTest
- spdlog and yaml-cpp linked through the `siv3d_template_core` library
- `compile_commands.json` exported to the project root for clangd / IntelliSense
- Visual Studio Code build, test, and debug configurations

## Requirements

| Dependency | Default location |
| --- | --- |
| OpenSiv3D 0.6.16 | `C:/Software/OpenSiv3D_0.6.16` |
| Sources of googletest, spdlog, yaml-cpp | `F:/codes/sources` |
| Visual Studio 2026 with the C++ workload | - |
| CMake 3.25+ and Ninja | - |

> **Static CRT:** Siv3D is built against the static CRT (`/MT`, `/MTd`).
> The template therefore builds googletest, spdlog, and yaml-cpp from source
> with the same runtime. Prebuilt `/MD` libraries under `F:/codes/third_party`
> are intentionally not used because mixing them with Siv3D could corrupt the heap.

Both paths can be overridden when configuring:

```bat
tools\cmake-msvc.cmd --preset ninja -DSIV3D_ROOT=<path> -DTHIRD_PARTY_SOURCE_ROOT=<path>
```

## Directory layout

```text
include/siv3d_template/   Public headers (.h)
src/                  Implementation files (.cc) and the Siv3D entry point (Main.cpp)
tests/                GoogleTest suites (.cc) and the test entry point (main.cpp)
App/                  Siv3D assets, Resource.rc, icon, and runtime data
tools/cmake-msvc.cmd  Loads the MSVC environment and forwards arguments to CMake
CMakeLists.txt        Build definition
CMakePresets.json     Configure, build, and test presets
```

File naming conventions:

- Headers end with `.h`.
- Implementation files end with `.cc`.
- Entry points (`src/Main.cpp`, `tests/main.cpp`) end with `.cpp`.

## Build and test

From any terminal:

```bat
tools\cmake-msvc.cmd --preset ninja
tools\cmake-msvc.cmd --build --preset debug
ctest --preset test-debug
```

For Release:

```bat
tools\cmake-msvc.cmd --build --preset release
ctest --preset test-release
```

If you are already in a "Developer Command Prompt for Visual Studio", plain
`cmake --preset ninja` and the matching `cmake --build` commands work as well.

The wrapper is only needed because Ninja must find `cl.exe`, which requires the
MSVC environment loaded by `vcvars64.bat`.

The application is written to `Intermediate/Siv3D_Template/<Config>/` and copied to
`App/`. Siv3D looks for `engine/` and application assets relative to the
executable, so always run the copy in `App/`.

`compile_commands.json` is generated in `build/` and copied to the project root
on every build.

## Visual Studio Code

- `F5` -> `Debug (MSVC)` launches the application.
- `F5` -> `Debug Tests (MSVC)` launches the test executable.
- `Ctrl+Shift+B` runs the default Debug build.
- "Tasks: Run Task" provides the build and test tasks for both configurations.

CMake Tools is configured to load the Visual Studio developer environment
automatically (`cmake.useVsDeveloperEnvironment`).

## Using this project as a template

1. Copy the project and delete `build/`, `Intermediate/`, and `.vs/`.
2. Rename the project and targets in `CMakeLists.txt`:
   `Siv3D_Template`, `siv3d_template_core`, `siv3d_template_tests`, and the `siv3d_template` namespace.
3. Adjust `SIV3D_ROOT` and `THIRD_PARTY_SOURCE_ROOT` in `CMakePresets.json`.
4. Keep `App/Resource.rc` and its Siv3D resource entries intact.
5. Add every new `.cc` file explicitly to the relevant CMake target; the build
   intentionally does not use file globbing.

## Third-party usage

`siv3d_template_core` links spdlog and yaml-cpp publicly, so any target that links
it can use them directly:

```cpp
#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
```

Sample modules are provided:

- `include/siv3d_template/version.h` / `src/version.cc` - application version
- `include/siv3d_template/logging.h` / `src/logging.cc` - spdlog logger factory
- `include/siv3d_template/config.h` / `src/config.cc` - yaml-cpp configuration loader

Corresponding tests live in `tests/`.
