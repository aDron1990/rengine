#pragma once

#include "Mesh.hpp"
#include "Texture.hpp"

#include <memory>

struct Renderer {
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture> texture;
};
