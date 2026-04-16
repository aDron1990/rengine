#pragma once

#include "OglMesh.hpp"
#include "graphics/RenderBackend.hpp"
#include "utils/types.hpp"
#include <cstdint>
#include <vector>

class OglRenderBackend : public RenderBackend {
public:
    void clear() noexcept override;

    MeshID createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept override;

    void draw(MeshID mesh) noexcept override;
    void draw(const std::vector<Line>& lines) noexcept override;

private:
    std::vector<OglMesh> m_meshes;
};
