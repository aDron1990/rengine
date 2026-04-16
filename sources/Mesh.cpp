#include "Mesh.hpp"

#include "third-party/tiny_obj_loader.h"

void Mesh::draw() const noexcept
{
    vao.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}
