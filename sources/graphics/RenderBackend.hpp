#pragma once

#include "Image.hpp"
#include "types.hpp"
#include "utils/types.hpp"
#include <cstdint>
#include <string>
#include <vector>

class RenderBackend {
public:
    ~RenderBackend() = default;
    virtual MeshID createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept = 0;
    virtual TextureID createTexture(const Image& image) noexcept = 0;
    virtual PipelineID createPipeline(const PipelineParams& params, const RenderState& state) noexcept = 0;

    virtual void bindTexture(TextureID texture, int slot = 0) noexcept = 0;
    virtual void bindPipeline(PipelineID pipeline) noexcept = 0;

    virtual void drawLines(const std::vector<Line>& lines) noexcept = 0;
    virtual void drawMesh(MeshID mesh) noexcept = 0;

    virtual void clear() noexcept = 0;
    virtual void setValue(PipelineID pipeline, const std::string& name, Value value) noexcept = 0;
};