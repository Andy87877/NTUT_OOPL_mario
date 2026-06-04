/**
 * @file InputHandler.cpp
 * @brief Implementation of InputHandler (Controller layer).
 *        Reads real-time keyboard states via SDL and applies to PlayerState.
 *        Key bindings match the C# reference (Right/D, Left/A, Space/Z, Down/S, E/LShift).
 *        Includes Simultaneous Opposite Cardinal Directions (SOCD) priority resolution
 *        and real-time event pumping to bypass VSync latency.
 * @inheritance IInputHandler -> InputHandler
 */
#include "Mario/Services/InputHandler.hpp"
#include "Mario/Services/KeyboardInputProfile.hpp"

#include <SDL.h>
#include "Mario/Services/Commands.hpp"

namespace Mario {

InputHandler::InputHandler(std::unique_ptr<IInputProfile> profile)
    : m_Profile(std::move(profile)) {
    if (!m_Profile) {
        m_Profile = std::make_unique<KeyboardInputProfile>();
    }
}

std::vector<std::shared_ptr<ICommand>> InputHandler::HandleInput(
    PlayerState& state, float speed, Level& level) {
    // -- Horizontal Movement Input Polling --
    bool rightHeld = m_Profile->IsButtonPressed(GameButton::RIGHT) || m_Profile->IsButtonDown(GameButton::RIGHT);
    bool leftHeld = m_Profile->IsButtonPressed(GameButton::LEFT) || m_Profile->IsButtonDown(GameButton::LEFT);

    // Detect new presses this frame using IsKeyDown
    bool rightPressed = m_Profile->IsButtonDown(GameButton::RIGHT);
    bool leftPressed = m_Profile->IsButtonDown(GameButton::LEFT);

    if (rightPressed) {
        m_LastDirectionPressed = 1;
    }
    if (leftPressed) {
        m_LastDirectionPressed = -1;
    }

    // Resolve simultaneous opposite directions (SOCD) with last-input priority
    if (rightHeld && leftHeld) {
        if (m_LastDirectionPressed == 1) {
            m_Right = true;
            m_Left = false;
        } else {
            m_Right = false;
            m_Left = true;
        }
    } else {
        m_Right = rightHeld;
        m_Left = leftHeld;
        if (!rightHeld && !leftHeld) {
            m_LastDirectionPressed = 0;
        }
    }

    // -- Jump & Crouch & Run Input Polling --
    m_Jump = m_Profile->IsButtonDown(GameButton::JUMP);

    m_Crouch = m_Profile->IsButtonPressed(GameButton::CROUCH) || m_Profile->IsButtonDown(GameButton::CROUCH);
    m_Run = m_Profile->IsButtonPressed(GameButton::RUN) || m_Profile->IsButtonDown(GameButton::RUN);

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
    bool fire = m_Profile->IsButtonDown(GameButton::FIRE);

    if (fire) {
        commands.push_back(std::make_shared<ShootFireballCommand>());
    }

    // -- Apply Physics Movement --
    commands.push_back(std::make_shared<ApplyPhysicsMovementCommand>(speed));

    return commands;
}

}  // namespace Mario
