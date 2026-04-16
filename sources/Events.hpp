#pragma once

#include <glm/glm.hpp>
namespace Event {

struct WindowClose { };

struct WindowResize {
    glm::ivec2 size;
};

}
