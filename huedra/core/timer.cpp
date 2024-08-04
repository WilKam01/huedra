#include "timer.hpp"
#include <chrono>

namespace huedra {

void Timer::init()
{
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    m_startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    m_currentTime = m_startTime;
    m_deltaTime = 0;
}

void Timer::update()
{
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    i64 now = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    m_deltaTime = now - m_currentTime;
    m_currentTime = now;
}

} // namespace huedra