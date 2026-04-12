# NuVelocity Engine

## Overview
NuVelocity is a C++ game engine, re-implementing Reflexive Entertainment's Velocity Engine. It powers games such as FreeRS42, providing modular 2D game development, asset management, rendering, audio, and scene management. Designed for extensibility, performance, and strong type safety.

## Features
- Modular engine architecture
- 2D rendering (SDL3-based)
- Asset and resource management
- Audio playback and management
- Scene and object system
- Type-safe object registration
- CMake-based build system

## Directory Structure
- `src/` — Engine source code
  - `model/` — Object model, property system
  - `system/` — Asset, audio, input, rendering, and utility systems
  - `third_party/` — External dependencies (SDL, zlib, physfs, etc.)

## Build Instructions
1. Install dependencies: SDL3, SDL3_image, SDL3_mixer, SDL3_ttf, zlib, physfs
2. Clone repository and submodules
3. Configure with CMake:
   ```sh
   cmake -S . -B build
   ```
4. Build:
   ```sh
   cmake --build build
   ```

## Usage
- Extend engine by adding new modules in `src/`
- Register new object types using the type-safe registration system

## License
Engine and main code are licensed under the Mozilla Public License 2.0 (MPL 2.0).
For third-party code, see the `third_party` directory for license information.

## Contact
For questions or contributions, open an issue or pull request.
