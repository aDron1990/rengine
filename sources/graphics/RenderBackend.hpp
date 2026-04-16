#pragma once

#include "types.hpp"
#include "utils/types.hpp"
#include <vector>

class RenderBackend {
public:
    ~RenderBackend() = default;
    virtual void draw(const std::vector<Line>& lines) noexcept = 0;
    virtual void draw(MeshID mesh) noexcept = 0;
    virtual void clear() noexcept = 0;

    virtual MeshID createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept = 0;
};