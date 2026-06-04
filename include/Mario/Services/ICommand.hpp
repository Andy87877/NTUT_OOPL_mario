/**
 * @file ICommand.hpp
 * @brief Abstract interface for input commands (Command Pattern).
 *        Allows decoupling user input devices/logic from player state actions.
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_COMMAND_HPP
#define MARIO_I_COMMAND_HPP

namespace Mario {

class PlayerState;
class Level;

/**
 * Interface representing a user action that can be executed on PlayerState.
 */
class ICommand {
   public:
    virtual ~ICommand() = default;
    
    /**
     * Executes the command on the target player state and level.
     * @param state The PlayerState Model to modify
     * @param level The current level block grid
     */
    virtual void Execute(PlayerState& state, Level& level) = 0;
};

}  // namespace Mario

#endif  // MARIO_I_COMMAND_HPP
