# AGENTS.md

## Project Overview

This repository contains `rengine`, a C++20 rendering/engine playground built with CMake and vcpkg.

The project is a Kerbal Space Program-like game with a custom engine. Its goal is to implement the core KSP-style mechanics from scratch, expand them with far-future technologies, and keep a strong focus on performance optimization.

Main areas:
- `sources/` - application, engine, systems, components, objects, and graphics code.
- `sources/graphics/opengl/` - OpenGL rendering backend implementation.
- `sources/graphics/vulkan/` - Vulkan rendering backend work.
- `sources/third-party/` - vendored third-party source files such as stb.
- `resources/` - shaders, models, images, fonts, and skybox assets used by the app.
- `documents/` - project notes for agents; document touched code areas here and link relevant notes from this file. See `documents/render-backend.md` before changing render backend interfaces. See `documents/origin-rebasing.md` before changing large-world/orbit position synchronization.

Core dependencies are declared in `vcpkg.json` and include GLEW, GLFW, GLM, EnTT, ImGui, Jolt, Assimp, doctest, Vulkan, Vulkan Memory Allocator, and vk-bootstrap.

## Development Commands

The project startup/build flow is documented in `README.md`. Use those commands as the source of truth.

Current build commands:
- Configure: `cmake -S . -B build --preset=debug-clang-cl`
- Build: `cmake --build build`

Useful project metric command from `README.md`:
- Count lines: `cloc . --exclude-dir=build,.vs,third-party,images,rocket`

## Working Rules

- Prefer small, focused changes.
- Follow existing code style and naming conventions.
- Do not refactor unrelated code.
- Do not change public APIs unless the task explicitly requires it.
- Preserve user changes already present in the working tree.
- When a task touches a meaningful code area, document the affected design, behavior, or workflow under `documents/` and add or update the corresponding link in this `AGENTS.md` file.

## Formatting

- Format C and C++ code with `clang-format`.
- The formatting style is defined in `.clang-format`; do not override it with ad hoc formatting choices.
- Keep generated build output, IDE state, and other local artifacts out of source changes.

## Naming and Code Style

- Use PascalCase for types, classes, structs, enums, and C++ filenames that define primary types, for example `RenderBackend`, `Transform`, `MeshRenderer`, and `OglTexture`.
- Use lowerCamelCase for functions and methods, for example `createMesh`, `bindTexture`, `getMatrix`, and `processInput`.
- Use `m_` prefixes for private data members, for example `m_registry`, `m_windowSize`, and `m_texture`.
- Prefer `enum class` for scoped enums.
- Handle identifiers use `ID` suffixes, for example `MeshID`, `TextureID`, `PipelineID`, and `RenderTextureID`.
- Keep headers guarded with `#pragma once`, matching the existing codebase.
- Prefer `noexcept` where surrounding code uses it for engine/backend operations.
- Follow existing include style: project headers use quoted includes, external/library headers use angle brackets.

## Testing and Validation

- After code changes, run the relevant build command from `README.md` when practical.
- Write focused tests for new or changed functionality when it is reasonable to test automatically, and run those tests before finishing.
- Tests use doctest and live under `tests/`; keep test CMake wiring in `tests/CMakeLists.txt`.
- For rendering, asset, shader, or backend changes, also consider whether the app needs a manual run to verify visual behavior.
- If a validation command cannot be run, explain the reason in the final response.

## Assets and Shaders

- Keep runtime assets under `resources/`.
- Keep GLSL shaders under `resources/shaders/`.
- Avoid renaming or moving assets unless all corresponding code paths and references are updated.

## Security and Repository Hygiene

- Do not commit secrets, credentials, machine-local paths, or private environment files.
- Do not edit vendored files in `sources/third-party/` unless the task specifically requires it.
- Avoid broad dependency changes unless they are necessary for the requested work.

## Final Response

- Summarize changed files and behavior.
- Mention validation commands run and any known gaps.
- Keep the response concise and focused on what changed.
