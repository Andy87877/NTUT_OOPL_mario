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
     * @param selection    0-based index of the currently highlighted item.
     * @param itemTexts    Dynamic display text for each option.
     * @param description  Dynamic description for the selected item.
     */
    void SetMenuContext(int selection, const std::vector<std::string>& itemTexts, const std::string& description);

    /**
     * Set whether the controls guide panel should be shown.
     */
    void SetShowControls(bool show) { m_ShowControls = show; }

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    int m_Selection = 0;
    std::string m_Description = "";
    bool m_ShowControls = false;
    int m_FrameCount = 0;

    std::shared_ptr<UIImage> m_Overlay;
    std::shared_ptr<UIText> m_PausedLabel;
    std::shared_ptr<UIText> m_DescLabel;
    std::shared_ptr<UIText> m_HintLabel;
    std::shared_ptr<UIImage> m_Cursor;
    std::vector<std::shared_ptr<UIText>> m_MenuTexts;

    // --- Controls Guide Widgets ---
    std::shared_ptr<UIText> m_ControlsTitle;
    std::shared_ptr<UIText> m_HeaderEng;
    std::shared_ptr<UIText> m_HeaderChi;
    std::shared_ptr<UIText> m_HeaderKey;
    std::vector<std::shared_ptr<UIText>> m_ControlsEng;
    std::vector<std::shared_ptr<UIText>> m_ControlsChi;
    std::vector<std::shared_ptr<UIText>> m_ControlsKey;
    std::shared_ptr<UIText> m_ControlsBackHint;
};

}  // namespace Mario

#endif  // MARIO_UI_ESC_MENU_PANEL_HPP
