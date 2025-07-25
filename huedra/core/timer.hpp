#pragma once

#include "core/types.hpp"

namespace huedra {

class Timer
{
public:
    const static u64 SECONDS_TO_NANO = 1'000'000'000;
    const static u64 MILLISECONDS_TO_NANO = 1'000'000;

    Timer() = default;
    ~Timer() = default;

    Timer(const Timer& rhs) = default;
    Timer& operator=(const Timer& rhs) = default;
    Timer(Timer&& rhs) = default;
    Timer& operator=(Timer&& rhs) = default;

    void init();
    void update();

    // Fixed interval in nanoseconds
    bool passedInterval(u64 fixedInterval, bool reset = true);
    void resetInterval();

    // In nanoseconds
    i64 currentTime() const { return m_currentTime; }
    i64 timeElapsed() const { return m_currentTime - m_startTime; }
    i64 dtNano() const { return m_deltaTime; }

    // In milliseconds
    float currentTimeMs() const { return static_cast<float>(m_currentTime) / static_cast<float>(MILLISECONDS_TO_NANO); }
    float elapsedMs() const
    {
        return static_cast<float>(m_currentTime - m_startTime) / static_cast<float>(MILLISECONDS_TO_NANO);
    }
    float dtMs() const { return static_cast<float>(m_deltaTime) / static_cast<float>(MILLISECONDS_TO_NANO); }

    // In seconds
    float currentTimeSeconds() const { return static_cast<float>(m_currentTime) / static_cast<float>(SECONDS_TO_NANO); }
    float elapsedSeconds() const
    {
        return static_cast<float>(m_currentTime - m_startTime) / static_cast<float>(SECONDS_TO_NANO);
    }
    float dt() const { return static_cast<float>(m_deltaTime) / static_cast<float>(SECONDS_TO_NANO); }

private:
    i64 m_startTime{};
    i64 m_currentTime{};
    i64 m_deltaTime{};
    i64 m_fixedIntervalTime{};
};

} // namespace huedra