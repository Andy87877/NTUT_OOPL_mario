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

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::shared_ptr<UIText> m_TitleLabel;
    std::shared_ptr<UIText> m_SubLabel;
};

}  // namespace Mario

#endif  // MARIO_UI_TITLE_PANEL_HPP
