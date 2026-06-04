/**
 * @file GameOverPanel.hpp
 * @brief Dedicated retro GameOver screen panel.
 *        Displays the game logo, the "GAME OVER" text, and the final score.
 * @inheritance IUIPanel <- GameOverPanel
 */
#ifndef MARIO_UI_GAME_OVER_PANEL_HPP
#define MARIO_UI_GAME_OVER_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

/**
 * GameOverPanel displays game over elements when player loses all lives.
 */
class GameOverPanel : public IUIPanel {
   public:
    GameOverPanel(const std::string& fontPath, int fontSize);
    ~GameOverPanel() override = default;

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_FrameCount = 0;

    std::shared_ptr<UIImage> m_Logo;
    std::shared_ptr<UIText> m_GameOverLabel;
    std::shared_ptr<UIText> m_ScoreLabel;
    std::shared_ptr<UIText> m_PromptLabel;
};

}  // namespace Mario

#endif  // MARIO_UI_GAME_OVER_PANEL_HPP
