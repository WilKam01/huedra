#include "timer.hpp"
#include <chrono>

namespace huedra {

void Timer::init()
{
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    m_startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    m_currentTime = m_startTime;
    m_deltaTime = 0;
    m_fixedIntervalTime = m_startTime;
}

void Timer::update()
{
    auto duration = std::chrono::high_resolution_clock::now().time_since_epoch();
    i64 now = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    m_deltaTime = now - m_currentTime;
    m_currentTime = now;
}

bool Timer::passedInterval(u64 fixedInterval, bool reset)
{
    bool passed = m_currentTime >= m_fixedIntervalTime + static_cast<i64>(fixedInterval);
    if (passed && reset)
    {
        m_fixedIntervalTime = m_currentTime;
    }
    return passed;
}

void Timer::resetInterval() { m_fixedIntervalTime = m_currentTime; }

} // namespace huedra