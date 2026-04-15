#include "Mesh.hpp"

void Mesh::draw() const noexcept
{
    vao.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
}
