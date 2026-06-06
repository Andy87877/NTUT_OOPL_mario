/**
 * @file FixedTimestep.cpp
 * @brief FixedTimestep helper implementation.
 * @inheritance None
 */
#include "Mario/Core/FixedTimestep.hpp"

namespace Mario {

FixedTimestep::FixedTimestep(double fixedStepMs, double maxFrameTimeMs)
    : m_FixedStep(fixedStepMs),
      m_MaxFrameTime(maxFrameTimeMs),
      m_Accumulator(0.0) {}

int FixedTimestep::Accumulate(double dtMs) {
    // Cap elapsed time to prevent a cascade of calculations during long lags
    if (dtMs > m_MaxFrameTime) {
        dtMs = m_MaxFrameTime;
    }

    m_Accumulator += dtMs;

    // Calculate how many discrete ticks of FIXED_STEP fit into the accumulated
    // time
    int steps = static_cast<int>(m_Accumulator / m_FixedStep);

    // Subtract the consumed time steps from the accumulator
    m_Accumulator -= steps * m_FixedStep;

    return steps;
}

}  // namespace Mario
