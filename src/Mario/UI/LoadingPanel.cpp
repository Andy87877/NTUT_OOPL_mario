#include "Mario/UI/LoadingPanel.hpp"

#include "Mario/Core/GameConfig.hpp"
#include "Util/Color.hpp"

namespace Mario {

LoadingPanel::LoadingPanel(const std::string& fontPath, int fontSize) {
    auto white = Util::Color::FromRGB(255, 255, 255);
    m_WorldLabel = std::make_shared<UIText>(fontPath, fontSize * 2, "", white);
    m_LivesText = std::make_shared<UIText>(fontPath, fontSize, "", white);

    // Pre-load the Mario preview sprite so the OpenGL texture is uploaded
    // before the first loading screen frame is drawn (avoids one-frame blank).
    std::string marioSpritePath =
        std::string(RESOURCE_DIR) + "/Sprites/MarioIdle.png";
    m_MarioPreview = std::make_shared<UIImage>(marioSpritePath);
    m_MarioPreview->SetPosition(-30.0f, -10.0f);
    m_MarioPreview->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};
    m_MarioPreview->SetZIndex(100.0f);  // Set ZIndex to 100.0f to match UIText and be on the exact same UI layer

    m_WorldLabel->SetVisible(false);
    m_LivesText->SetVisible(false);
    m_MarioPreview->SetVisible(false);

    // Resolve Traditional Chinese font fallback path
    std::string chineseFontPath = GameConfig::GetChineseFontPath(fontPath);
    m_CreditLabel = std::make_shared<UIText>(
        chineseFontPath, fontSize * 0.8f, "113820033 電資二 謝奕宏", white);
    m_CreditLabel->SetVisible(false);
}

void LoadingPanel::Register(Util::Renderer& renderer) {
    renderer.AddChild(m_WorldLabel);
    renderer.AddChild(m_LivesText);
    renderer.AddChild(m_MarioPreview);
    renderer.AddChild(m_CreditLabel);
}

void LoadingPanel::Show() {
    m_WorldLabel->SetVisible(true);
    m_LivesText->SetVisible(true);
    m_MarioPreview->SetVisible(true);
    m_CreditLabel->SetVisible(true);
}

void LoadingPanel::Hide() {
    m_WorldLabel->SetVisible(false);
    m_LivesText->SetVisible(false);
    m_MarioPreview->SetVisible(false);
    m_CreditLabel->SetVisible(false);
}

void LoadingPanel::Refresh(const GameStateManager& gs) {
    m_WorldLabel->SetTextContent("WORLD " + gs.GetLevelName());
    m_WorldLabel->SetPosition(0.0f, 50.0f);

    int lives = gs.GetLives();
    std::string livesStr =
        "x " + std::string(lives < 10 ? "0" : "") + std::to_string(lives);
    m_LivesText->SetTextContent(livesStr);
    m_LivesText->SetPosition(30.0f, -10.0f);

    // Ensure the Mario preview is placed exactly inside the blue box on the left of "x 03"
    m_MarioPreview->SetPosition(-30.0f, -10.0f);
    m_MarioPreview->m_Transform.scale = {GameConfig::DRAW_SCALE, GameConfig::DRAW_SCALE};

    m_CreditLabel->SetPosition(0.0f, -260.0f);
}

}  // namespace Mario
