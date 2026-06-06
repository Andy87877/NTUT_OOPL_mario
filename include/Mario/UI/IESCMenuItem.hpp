/**
 * @file IESCMenuItem.hpp
 * @brief Abstract interface representing a single menu option in the Pause (ESC) Menu.
 *        Implements the Command Pattern / Strategy Pattern to decouple actions from
 *        the ESCMenuSceneHandler and ESCMenuPanel classes.
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_ESC_MENU_ITEM_HPP
#define MARIO_I_ESC_MENU_ITEM_HPP

#include <string>

class App;

namespace Mario {

class ESCMenuSceneHandler;

/**
 * @brief Command interface for a pause menu item.
 */
class IESCMenuItem {
   public:
    virtual ~IESCMenuItem() = default;

    /**
     * @brief Gets the display text for this menu item.
     * @param app The main App context
     * @return Display text string
     */
    virtual std::string GetDisplayText(App& app) const = 0;

    /**
     * @brief Gets the context-sensitive help/description text for this item.
     * @return Description text string
     */
    virtual std::string GetDescriptionText() const = 0;

    /**
     * @brief Executes the action associated with this menu item.
     * @param app The main App context
     * @param handler The active ESCMenuSceneHandler context
     */
    virtual void Execute(App& app, ESCMenuSceneHandler& handler) = 0;
};

}  // namespace Mario

#endif  // MARIO_I_ESC_MENU_ITEM_HPP
