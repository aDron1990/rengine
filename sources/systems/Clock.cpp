#include "Clock.hpp"

void Clock::update() noexcept
{
    auto now = std::chrono::steady_clock::now();
    m_delta = std::chrono::duration<float>(now - m_lastUpdate).count();
    m_lastUpdate = now;
}

float Clock::getDelta() noexcept
{
    return m_delta;
}