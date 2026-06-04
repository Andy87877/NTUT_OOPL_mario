/**
 * @file AxeEndingPanel.hpp
 * @brief Axe ending panel for the victory credits scene in level 8-4.
 * @inheritance IUIPanel <- AxeEndingPanel
 */
#ifndef MARIO_UI_AXE_ENDING_PANEL_HPP
#define MARIO_UI_AXE_ENDING_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class AxeEndingPanel : public IUIPanel {
   public:
    AxeEndingPanel(const std::string& fontPath, int fontSize);
    ~AxeEndingPanel() override = default;

    /** Set whether credits text should be visible before calling Refresh(). */
    void SetShowCredits(bool show) { m_ShowCredits = show; }

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    bool m_ShowCredits = false;
    std::shared_ptr<UIText> m_Line1;
    std::shared_ptr<UIText> m_Line2;
};

}  // namespace Mario

#endif  // MARIO_UI_AXE_ENDING_PANEL_HPP
