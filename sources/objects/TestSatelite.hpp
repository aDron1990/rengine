#pragma once

#include "ModelObject.hpp"
#include <entt/entity/fwd.hpp>

class TestSatelite : public ModelObject {
public:
    TestSatelite(entt::registry& regisrty, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture, std::shared_ptr<Texture> specular);
    void update() noexcept;

};