/**
 * @file TitlePanel.hpp
 * @brief Title panel for the start menu.
 * @inheritance IUIPanel <- TitlePanel
 */
#ifndef MARIO_UI_TITLE_PANEL_HPP
#define MARIO_UI_TITLE_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class TitlePanel : public IUIPanel {
   public:
    TitlePanel(const std::string& fontPath, int fontSize);
    ~TitlePanel() override = default;

    /**
     * Set the current menu selection index before Refresh().
     * @param selection 0 = 1 Player Game, 1 = Quit Game
     */
    void SetMenuContext(int selection);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_Selection = 0;
    int m_FrameCount = 0;

    std::shared_ptr<UIImage> m_Logo;
    std::shared_ptr<UIText> m_OnePlayerLabel;
    std::shared_ptr<UIText> m_QuitLabel;
    std::shared_ptr<UIImage> m_Cursor;
    std::shared_ptr<UIText> m_SubLabel;
};

}  // namespace Mario

#endif  // MARIO_UI_TITLE_PANEL_HPP
