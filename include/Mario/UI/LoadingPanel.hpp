/**
 * @file LoadingPanel.hpp
 * @brief Loading screen panel showing world name, lives remaining, and Mario preview.
 * @inheritance IUIPanel <- LoadingPanel
 */
#ifndef MARIO_UI_LOADING_PANEL_HPP
#define MARIO_UI_LOADING_PANEL_HPP

#include <memory>
#include <string>

#include "Mario/UI/UIPanel.hpp"
#include "Mario/UI/UIWidgets.hpp"
#include "Util/Renderer.hpp"

namespace Mario {

class LoadingPanel : public IUIPanel {
   public:
    LoadingPanel(const std::string& fontPath, int fontSize);
    ~LoadingPanel() override = default;

    void Register(Util::Renderer& renderer) override;
    void Show() override;
    void Hide() override;
    void Refresh(const GameStateManager& gs) override;

   private:
    std::shared_ptr<UIText> m_WorldLabel;
    std::shared_ptr<UIText> m_LivesText;
    std::shared_ptr<UIImage> m_MarioPreview;
};

}  // namespace Mario

#endif  // MARIO_UI_LOADING_PANEL_HPP
