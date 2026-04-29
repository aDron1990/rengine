# Render Backend Split

This renderer separates backend responsibilities into three interfaces declared in `sources/graphics/RenderBackend.hpp`.

## RenderBackend

`RenderBackend` is the entry point owned by engine systems. It does not create resources or issue draw calls directly.

- Use `getDevice()` to access resource creation and resource metadata.
- Use `getCommandBuffer()` to access framebuffer binding, pipeline/resource binding, clears, uniforms, and draw calls.

Keep long-lived backend ownership as `std::shared_ptr<RenderBackend>` when systems need to share the selected backend implementation.

## RenderDevice

`RenderDevice` owns API resources and returns renderer handle IDs.

- Creates meshes, textures, cubemaps, pipelines, and render textures.
- Exposes render texture metadata needed by UI and layer code.
- Tracks default framebuffer size through `resizeDefaultFramebuffer()`.

Resource upload code, asset loading integration, and render layer allocation should go through `RenderBackend::getDevice()`.

## CommandBuffer

`CommandBuffer` contains render commands and transient binding state.

- Binds textures, render textures, framebuffers, and pipelines.
- Clears color/depth targets.
- Sets pipeline uniform values.
- Issues mesh, line, and cubemap draw calls.

Render passes should request a command buffer at the beginning of render code:

```cpp
auto& commandBuffer = backend.getCommandBuffer();
commandBuffer.bindPipeline(pipeline);
commandBuffer.drawMesh(meshID);
```

## OpenGL Implementation

The OpenGL backend implements the split in `sources/graphics/opengl/OglRenderBackend.hpp` and `.cpp`.

- `OglRenderDevice` owns `OglMesh`, `OglTexture`, `OglRenderTexture`, `OglPipeline`, and `OglCubemap` storage.
- `OglCommandBuffer` references `OglRenderDevice` and translates render commands to OpenGL calls.
- `OglRenderBackend` owns one device and one command buffer and exposes them through the generic backend interface.

When adding new rendering features, put persistent resource allocation on `RenderDevice` and per-frame API commands on `CommandBuffer`.
