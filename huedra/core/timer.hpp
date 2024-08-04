#pragma once

#include "core/types.hpp"

namespace huedra {

class Timer
{
private:
    const static u64 SECONDS_TO_NANO = 1'000'000'000;

public:
    Timer() = default;
    ~Timer() = default;

    void init();
    void update();

    // In nanoseconds
    i64 timeElapsed() { return m_currentTime - m_startTime; }
    i64 dtNano() { return m_deltaTime; }

    // In seconds
    float secondsElapsed() { return (m_currentTime - m_startTime) / static_cast<float>(SECONDS_TO_NANO); }
    float dt() { return m_deltaTime / static_cast<float>(SECONDS_TO_NANO); }

private:
    i64 m_startTime;
    i64 m_currentTime;
    i64 m_deltaTime;
};

} // namespace huedra