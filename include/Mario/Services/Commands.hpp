/**
 * @file Commands.hpp
 * @brief Concrete implementations of input commands (Command Pattern).
 * @inheritance ICommand -> MoveRightCommand, MoveLeftCommand, StopHorizontalMovementCommand,
 *             JumpCommand, CrouchCommand, StandUpCommand, RunCommand, StopRunningCommand,
 *             ShootFireballCommand, ApplyPhysicsMovementCommand
 */
#ifndef MARIO_COMMANDS_HPP
#define MARIO_COMMANDS_HPP

#include "Mario/Services/ICommand.hpp"

namespace Mario {

class MoveRightCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class MoveLeftCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class StopMovingRightCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class StopMovingLeftCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class StopHorizontalMovementCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class JumpCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class CrouchCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class StandUpCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class RunCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class StopRunningCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class ShootFireballCommand : public ICommand {
   public:
    void Execute(PlayerState& state, Level& level) override;
};

class ApplyPhysicsMovementCommand : public ICommand {
   public:
    explicit ApplyPhysicsMovementCommand(float speed) : m_Speed(speed) {}
    void Execute(PlayerState& state, Level& level) override;

   private:
    float m_Speed;
};

}  // namespace Mario

#endif  // MARIO_COMMANDS_HPP
