/**
 * @file ESCMenuPanel.hpp
 * @brief Pause menu panel overlay supporting stage warp and cheat mode selections.
 * @inheritance IUIPanel <- ESCMenuPanel
 */
#ifndef MARIO_UI_ESC_MENU_PANEL_HPP
#define MARIO_UI_ESC_MENU_PANEL_HPP

#include <memory>
#include <string>
#include <vector>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class ESCMenuPanel : public IUIPanel {
   public:
    ESCMenuPanel(const std::string& fontPath, int fontSize);
    ~ESCMenuPanel() override = default;

    /**
     * Supply menu-specific context before calling Refresh().
     * @param selection      0-based index of the currently highlighted item.
     * @param powerStateName Current cheat power name ("SMALL"/"BIG"/etc.).
     */
    void SetMenuContext(int selection, const std::string& powerStateName);

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_Selection = 0;
    std::string m_PowerStateName = "SMALL";

    std::shared_ptr<UIText> m_PausedLabel;
    std::vector<std::shared_ptr<UIText>> m_MenuTexts;
};

}  // namespace Mario

#endif  // MARIO_UI_ESC_MENU_PANEL_HPP
