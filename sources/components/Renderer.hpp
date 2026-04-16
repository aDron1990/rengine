#pragma once

#include "Model.hpp"
#include "Texture.hpp"

#include <memory>

struct Renderer {
    std::shared_ptr<Model> model;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<Texture> specular;
    bool drawAABB = false;
};

struct Transparent { };