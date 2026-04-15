#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Line {
    glm::vec3 p1;
    glm::vec3 p2;
};

class LineBatch {
public:
    void push(const Line& line) noexcept;
    void draw() const noexcept;

private:
    std::vector<Line> m_lines;
};
