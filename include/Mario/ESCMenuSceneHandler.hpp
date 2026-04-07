/**
 * @file ESCMenuSceneHandler.hpp
 * @brief Pause/ESC menu scene handler - shows pause menu with level selection.
 * @inheritance ISceneHandler
 */
#ifndef MARIO_ESC_MENU_SCENE_HANDLER_HPP
#define MARIO_ESC_MENU_SCENE_HANDLER_HPP

#include "Mario/GameStateManager.hpp"
#include "Mario/ISceneHandler.hpp"

namespace Mario {

/**
 * Manages the ESC menu (pause menu).
 * Allows player to:
 * - Resume game
 * - Jump to 1-1, 1-2, or 8-4
 */
class ESCMenuSceneHandler : public ISceneHandler {
   public:
    ESCMenuSceneHandler(GameStateManager* gameState);
    virtual ~ESCMenuSceneHandler() = default;

    void OnEnter() override;
    bool Update() override;
    void OnExit() override;

    const char* GetNextSceneName() const override;
    const char* GetName() const override { return "ESCMenuScene"; }

    /**
     * Get current menu selection (for rendering).
     * 0 = Resume, 1 = 1-1, 2 = 1-2, 3 = 8-4
     */
    int GetSelection() const { return m_Selection; }

   private:
    GameStateManager* m_GameState;
    int m_Selection = 0;  // Current highlighted menu item
    const char* m_NextScene = nullptr;
};

}  // namespace Mario

#endif  // MARIO_ESC_MENU_SCENE_HANDLER_HPP
