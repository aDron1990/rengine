#pragma once

#include <vector>

#include "BoundingBox.hpp"
#include "AABB.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "utils.hpp"

struct Mesh {
    void draw() const noexcept;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VertexBuffer vbo;
    VertexArray vao;
    IndexBuffer ibo;
    BoundingBox aabb;
};
