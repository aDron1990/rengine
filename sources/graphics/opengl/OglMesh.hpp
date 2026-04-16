#pragma once

#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"

struct OglMesh {
    VertexBuffer vbo;
    VertexArray vao;
    IndexBuffer ibo;
    size_t verticesCount = 0;
    size_t indicesCount = 0;
};
