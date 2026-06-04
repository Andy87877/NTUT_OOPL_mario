/**
 * @file FixedTimestep.hpp
 * @brief Class encapsulating the fixed-timestep accumulator loop.
 *        Decouples the physics time-accumulation math from the main App controller
 *        to respect Single Responsibility (SRP) and keep App.cpp clean.
 * @inheritance None (standalone helper utility)
 */
#ifndef MARIO_FIXED_TIMESTEP_HPP
#define MARIO_FIXED_TIMESTEP_HPP

namespace Mario {

/**
 * Manages game loop timing by accumulating elapsed frame time (dt)
 * and dividing it into discrete, fixed physics ticks to prevent game speed
 * fluctuation on different computers.
 */
class FixedTimestep {
   public:
    /**
     * @param fixedStepMs  Duration of a single physics tick in milliseconds (default 20ms = 50 FPS).
     * @param maxFrameTimeMs Hard limit to discard excess frame time and prevent the "spiral of death" during lag spikes.
     */
    explicit FixedTimestep(double fixedStepMs = 20.0, double maxFrameTimeMs = 100.0);
    ~FixedTimestep() = default;

    /**
     * Accumulate actual elapsed frame time and calculate how many physics ticks
     * should run in the current update loop.
     * @param dtMs Actual elapsed frame duration in milliseconds.
     * @return Number of fixed updates to execute.
     */
    int Accumulate(double dtMs);

   private:
    double m_FixedStep;
    double m_MaxFrameTime;
    double m_Accumulator;
};

}  // namespace Mario

#endif  // MARIO_FIXED_TIMESTEP_HPP
