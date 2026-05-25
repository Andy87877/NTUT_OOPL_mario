/**
 * @file CoinUI.cpp
 * @brief Implementation of coin display UI.
 * @inheritance None (Composite component)
 */
#include "Mario/UI/CoinUI.hpp"

#include <cstdio>

#include "Util/Color.hpp"
#include "config.hpp"

namespace Mario {

CoinUI::CoinUI(const std::string& fontPath, int fontSize, float screenX,
               float screenY, float coinScale)
    : m_ScreenX(screenX), m_ScreenY(screenY), m_CoinScale(coinScale) {
    auto colorWhite = Util::Color::FromRGB(255, 255, 255);

    // Initialize coin image with first frame
    std::string coinPath =
        std::string(RESOURCE_DIR) + COIN_SPRITE_BASE + "1.png";
    m_CoinImage = std::make_shared<UIImage>(coinPath);

    // Set coin image scale (2x by default)
    m_CoinImage->m_Transform.scale = {m_CoinScale, m_CoinScale};

    // Initialize count text (will be updated in Update())
    m_CountText =
        std::make_shared<UIText>(fontPath, fontSize, "x00", colorWhite);

    // Position elements
    // Coin icon: position adjusted for scale and moved left
    // With 2x scale, coin is wider, so offset position more to the left
    float coinScreenX = screenX - 10.0f;  // Shift left a bit
    m_CoinImage->SetPosition(ScreenXToPTSD(coinScreenX),
                             ScreenYToPTSD(screenY));

    // Text: offset to the right of coin icon
    // With 2x scale, coin sprite is ~64 pixels wide (32 * 2), add margin
    float textOffsetX = coinScreenX + 60.0f;
    m_CountText->SetPosition(ScreenXToPTSD(textOffsetX),
                             ScreenYToPTSD(screenY));
}

void CoinUI::Update(int coinCount) {
    // Increment animation counter
    m_AnimationCounter++;

    // Update animation frame if counter exceeds interval
    if (m_AnimationCounter >= FRAME_INTERVAL) {
        m_AnimationCounter = 0;
        m_CurrentFrame = (m_CurrentFrame + 1) % COIN_FRAME_COUNT;
        UpdateCoinSprite();
    }

    // Update coin count text
    char countStr[10];
    snprintf(countStr, sizeof(countStr), "x%02d", coinCount % 100);
    m_CountText->SetTextContent(countStr);
}

float CoinUI::ScreenXToPTSD(float screenX) const {
    // Convert from screen coords (0-1280) to PTSD coords (-640 to 640)
    return screenX - 640.0f;
}

float CoinUI::ScreenYToPTSD(float screenY) const {
    // Convert from screen coords (0-720) to PTSD coords (360 to -360)
    return 360.0f - screenY;
}

void CoinUI::UpdateCoinSprite() {
    // Build path: CoinTop1.png, CoinTop2.png, CoinTop3.png
    // m_CurrentFrame is 0, 1, 2 -> frame number is 1, 2, 3
    int frameNum = m_CurrentFrame + 1;

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s%s%d.png", RESOURCE_DIR,
             COIN_SPRITE_BASE, frameNum);

    m_CoinImage->SetImagePath(buffer);
}

}  // namespace Mario
