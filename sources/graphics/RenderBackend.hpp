#pragma once

#include "Image.hpp"
#include "types.hpp"
#include "utils/types.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class RenderBackend {
public:
    ~RenderBackend() = default;
    virtual MeshID createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept = 0;
    virtual TextureID createTexture(const Image& image) noexcept = 0;
    virtual PipelineID createPipeline(const PipelineParams& params, const RenderState& state) noexcept = 0;
    virtual CubemapID createCubemap(const std::vector<Image>& images) noexcept = 0;
    virtual RenderTextureID createRenderTexture(uint32_t width, uint32_t height) noexcept = 0;

    virtual glm::ivec2 getRenderTextureSize(RenderTextureID texture) noexcept = 0;
    virtual size_t getGuiTexture(RenderTextureID texture) noexcept = 0;

    virtual void bindTexture(TextureID texture, int slot = 0) noexcept = 0;
    virtual void bindRenderTexture(RenderTextureID texture, int slot = 0) noexcept = 0;
    virtual void bindFramebuffer(RenderTextureID texture) noexcept = 0;
    virtual void bindDefaultFramebuffer() noexcept = 0;
    virtual void bindPipeline(PipelineID pipeline) noexcept = 0;

    virtual void resizeDefaultFramebuffer(uint32_t width, uint32_t height) noexcept = 0;

    virtual void drawLines(const std::vector<Line>& lines) noexcept = 0;
    virtual void drawMesh(MeshID mesh) noexcept = 0;
    virtual void drawCubemap(CubemapID cubemap) noexcept = 0;

    virtual void clearColor() noexcept = 0;
    virtual void clearDepth() noexcept = 0;
    virtual void setValue(PipelineID pipeline, const std::string& name, Value value) noexcept = 0;
};