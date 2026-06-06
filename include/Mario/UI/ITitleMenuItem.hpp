/**
 * @file ITitleMenuItem.hpp
 * @brief Abstract interface representing a single menu option in the Title Screen Menu.
 *        Implements the Command Pattern to decouple actions from TitleSceneHandler and TitlePanel.
 * @inheritance None (pure interface)
 */
#ifndef MARIO_I_TITLE_MENU_ITEM_HPP
#define MARIO_I_TITLE_MENU_ITEM_HPP

#include <string>

class App;

namespace Mario {

class TitleSceneHandler;

/**
 * @brief Command interface for a title menu item.
 */
class ITitleMenuItem {
   public:
    virtual ~ITitleMenuItem() = default;

    /**
     * @brief Gets the display text for this menu item.
     * @param app The main App context
     * @return Display text string
     */
    virtual std::string GetDisplayText(App& app) const = 0;

    /**
     * @brief Executes the action associated with this menu item.
     * @param app The main App context
     * @param handler The active TitleSceneHandler context
     */
    virtual void Execute(App& app, TitleSceneHandler& handler) = 0;
};

}  // namespace Mario

#endif  // MARIO_I_TITLE_MENU_ITEM_HPP
