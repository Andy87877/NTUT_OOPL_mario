/**
 * @file UIPanel.hpp
 * @brief Strategy interface for UI panels.
 * @inheritance IUIPanel (interface root)
 */
#ifndef MARIO_UI_PANEL_HPP
#define MARIO_UI_PANEL_HPP

#include "Mario/Level/GameStateManager.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

/**
 * Interface for a single game-screen's UI panel.
 * Each concrete panel owns its UIText/UIImage widgets and is responsible
 * for registering them into the renderer, showing/hiding them, and refreshing
 * their content from the game state.
 */
class IUIPanel {
   public:
    virtual ~IUIPanel() = default;

    /** Register all child widgets into the renderer once at startup. */
    virtual void Register(Util::Renderer& renderer) = 0;

    /** Make all owned widgets visible. */
    virtual void Show() = 0;

    /** Hide all owned widgets. */
    virtual void Hide() = 0;

    /** Refresh text content and layout from the current game state. */
    virtual void Refresh(const GameStateManager& gs) = 0;
};

}  // namespace Mario

#endif  // MARIO_UI_PANEL_HPP
