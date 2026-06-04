/**
 * @file SimpleTextPanel.hpp
 * @brief Reusable simple text panel for Game Over and World Victory screens.
 * @inheritance IUIPanel <- SimpleTextPanel
 */
#ifndef MARIO_UI_SIMPLE_TEXT_PANEL_HPP
#define MARIO_UI_SIMPLE_TEXT_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class SimpleTextPanel : public IUIPanel {
   public:
    SimpleTextPanel(const std::string& fontPath, int fontSize,
                    const std::string& titleText);
    ~SimpleTextPanel() override = default;

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::string m_TitleText;
    std::shared_ptr<UIText> m_TitleLabel;
    std::shared_ptr<UIText> m_ScoreText;
};

}  // namespace Mario

#endif  // MARIO_UI_SIMPLE_TEXT_PANEL_HPP
