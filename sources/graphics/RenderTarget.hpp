#pragma once

#include <glm/glm.hpp>

class RenderTarget {
public:
    enum class Type {
        Texture,
        Window
    };

public:
    virtual ~RenderTarget() = default;
    virtual Type getType() const noexcept = 0;
    virtual glm::ivec2 getSize() const noexcept = 0;
    float getAspect() const noexcept
    {
        auto size = getSize();
        return static_cast<float>(size.x) / static_cast<float>(size.y);
    }
};
