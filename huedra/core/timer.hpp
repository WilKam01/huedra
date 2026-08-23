#pragma once

#include "core/constants.hpp"
#include "core/types.hpp"

namespace huedra {

class Timer
{
public:
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
    f32 currentTimeMs() const
    {
        return static_cast<f32>(m_currentTime) / static_cast<f32>(constants::MILLISECONDS_TO_NANO);
    }
    f32 elapsedMs() const
    {
        return static_cast<f32>(m_currentTime - m_startTime) / static_cast<f32>(constants::MILLISECONDS_TO_NANO);
    }
    f32 dtMs() const { return static_cast<f32>(m_deltaTime) / static_cast<f32>(constants::MILLISECONDS_TO_NANO); }

    // In seconds
    f32 currentTimeSeconds() const
    {
        return static_cast<f32>(m_currentTime) / static_cast<f32>(constants::SECONDS_TO_NANO);
    }
    f32 elapsedSeconds() const
    {
        return static_cast<f32>(m_currentTime - m_startTime) / static_cast<f32>(constants::SECONDS_TO_NANO);
    }
    f32 dt() const { return static_cast<f32>(m_deltaTime) / static_cast<f32>(constants::SECONDS_TO_NANO); }

private:
    i64 m_startTime{};
    i64 m_currentTime{};
    i64 m_deltaTime{};
    i64 m_fixedIntervalTime{};
};

} // namespace huedra