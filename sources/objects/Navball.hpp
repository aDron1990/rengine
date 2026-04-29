#pragma once

#include "Model.hpp"
#include "ModelObject.hpp"
#include "graphics/types.hpp"
#include <entt/entity/fwd.hpp>
#include <memory>

class Navball : public ModelObject {
public:
    Navball(entt::registry& registry, std::shared_ptr<Model> model, TextureID texture);

    void update() noexcept override;
    glm::quat getIndicatorsQuat() const noexcept;
    int getRenderLayer() const noexcept { return m_renderlayer; }

private:
    entt::entity m_cameraEntity;
    entt::entity m_debugLines;
    int m_renderlayer;
};
