#pragma once

#include "BoundingBox.hpp"
#include "Buffer.hpp"
#include "VertexArray.hpp"
#include "utils.hpp"

class Mesh {
public:
    Mesh(const std::string& objPath);
    void draw() const noexcept;
    BoundingBox getAABB() const noexcept;

private:
    void loadObj(const std::string& objPath);

private:
    std::vector<Vertex> m_vertices;
    VertexBuffer m_vbo;
    VertexArray m_vao;
    BoundingBox m_aabb;
};
