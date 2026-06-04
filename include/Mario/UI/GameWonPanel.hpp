/**
 * @file GameWonPanel.hpp
 * @brief High-fidelity retro victory screen panel.
 *        Displays game logo, congratulatory messages, final score, and
 *        sprites of Mario and Princess Peach facing each other.
 * @inheritance IUIPanel <- GameWonPanel
 */
#ifndef MARIO_UI_GAME_WON_PANEL_HPP
#define MARIO_UI_GAME_WON_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

/**
 * GameWonPanel displays the final victory details once all levels are completed.
 * Features an engaging layout with the main logo, player sprites, and final score.
 */
class GameWonPanel : public IUIPanel {
   public:
    GameWonPanel(const std::string& fontPath, int fontSize);
    ~GameWonPanel() override = default;

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_FrameCount = 0;

    std::shared_ptr<UIImage> m_Logo;
    std::shared_ptr<UIText> m_VictoryLabel;
    std::shared_ptr<UIImage> m_MarioImage;
    std::shared_ptr<UIImage> m_PrincessImage;
    std::shared_ptr<UIText> m_ScoreLabel;
    std::shared_ptr<UIText> m_MessageLabel;
    std::shared_ptr<UIText> m_CreditLabel;
    std::shared_ptr<UIText> m_PromptLabel;
};

}  // namespace Mario

#endif  // MARIO_UI_GAME_WON_PANEL_HPP
