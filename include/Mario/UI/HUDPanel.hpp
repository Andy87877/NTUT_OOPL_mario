/**
 * @file HUDPanel.hpp
 * @brief HUD panel displaying game stats like score, level name, time, and coins.
 * @inheritance IUIPanel <- HUDPanel
 */
#ifndef MARIO_UI_HUD_PANEL_HPP
#define MARIO_UI_HUD_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/CoinUI.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Mario/Level/GameStateManager.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class HUDPanel : public IUIPanel {
   public:
    HUDPanel(const std::string& fontPath, int fontSize);
    ~HUDPanel() override = default;

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::string m_FontPath;
    int m_FontSize;

    std::shared_ptr<UIText> m_HeaderMario;
    std::shared_ptr<UIText> m_HeaderWorld;
    std::shared_ptr<UIText> m_HeaderTime;
    std::shared_ptr<UIText> m_ScoreText;
    std::shared_ptr<UIText> m_WorldText;
    std::shared_ptr<UIText> m_TimeText;
    std::shared_ptr<CoinUI> m_CoinUI;

    int m_FlashCounter = 0;
};

}  // namespace Mario

#endif  // MARIO_UI_HUD_PANEL_HPP
