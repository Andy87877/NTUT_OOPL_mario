/**
 * @file MockInputHandler.cpp
 * @brief Implementation of MockInputHandler (Controller layer).
 *        Translates programmatic mock inputs into PlayerState commands.
 * @inheritance IInputHandler -> MockInputHandler
 */
#include "Mario/Services/MockInputHandler.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Services/Commands.hpp"

namespace Mario {

std::vector<std::shared_ptr<ICommand>> MockInputHandler::HandleInput(
    PlayerState& state, float speed, Level& level) {
    std::vector<std::shared_ptr<ICommand>> commands;

    if (!state.IsControllable() || state.IsDead()) {
        commands.push_back(std::make_shared<StopHorizontalMovementCommand>());
        return commands;
    }

    // -- Horizontal Movement Commands --
    if (m_Right) {
        commands.push_back(std::make_shared<MoveRightCommand>());
    } else {
        commands.push_back(std::make_shared<StopMovingRightCommand>());
    }

    if (m_Left) {
        commands.push_back(std::make_shared<MoveLeftCommand>());
    } else {
        commands.push_back(std::make_shared<StopMovingLeftCommand>());
    }

    // -- Jump --
    if (m_Jump) {
        commands.push_back(std::make_shared<JumpCommand>());
    }

    // -- Crouch --
    if (m_Crouch) {
        commands.push_back(std::make_shared<CrouchCommand>());
    } else {
        commands.push_back(std::make_shared<StandUpCommand>());
    }

    // -- Run --
    if (m_Run) {
        commands.push_back(std::make_shared<RunCommand>());
    } else {
        commands.push_back(std::make_shared<StopRunningCommand>());
    }

    // -- Fire --
    if (m_Fire) {
        commands.push_back(std::make_shared<ShootFireballCommand>());
    }

    // -- Apply Physics Movement --
    commands.push_back(std::make_shared<ApplyPhysicsMovementCommand>(speed));

    return commands;
}

}  // namespace Mario
