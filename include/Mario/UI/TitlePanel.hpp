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
     * Supply title menu context dynamically.
     * @param selection    Highlighted menu index.
     * @param itemTexts    Texts for all title screen options.
     */
    void SetMenuContext(int selection, const std::vector<std::string>& itemTexts);

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
    int m_FrameCount = 0;
    bool m_ShowControls = false;

    // --- Standard Title Screen Widgets ---
    std::shared_ptr<UIImage> m_Logo;
    std::vector<std::shared_ptr<UIText>> m_MenuTexts;
    std::shared_ptr<UIImage> m_Cursor;
    std::shared_ptr<UIText> m_SubLabel;
    std::shared_ptr<UIText> m_CreditLabel;

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

#endif  // MARIO_UI_TITLE_PANEL_HPP
