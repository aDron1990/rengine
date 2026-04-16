#pragma once

#include <chrono>

class Clock {
public:
    void update() noexcept;
    float getDelta() noexcept;

private:
    std::chrono::steady_clock::time_point m_lastUpdate = std::chrono::steady_clock::now();
    float m_delta = 0;
};