/**
 * @file FloatingText.cpp
 * @brief Logic for floating text effect.
 * @inheritance None (Wrapper with composition)
 */
#include "Mario/FloatingText.hpp"

#include "Util/Color.hpp"

namespace Mario {

FloatingText::FloatingText(float worldX, float worldY, const std::string& text,
                           int durationFrames)
    : m_LifetimeCounter(durationFrames), m_OriginalDuration(durationFrames) {
    m_CurrentPosition = {worldX, worldY};
    m_UIText =
        std::make_shared<UIText>(std::string(RESOURCE_DIR) + "/Font/mario.ttf",
                                 16, text, Util::Color::FromRGB(255, 255, 255));
    m_UIText->SetPosition(worldX, worldY);
}

void FloatingText::Update() {
    m_LifetimeCounter--;

    if (m_LifetimeCounter > 0) {
        // Float upward 1 pixel every frame
        m_CurrentPosition.y += 1.0f;
        m_UIText->SetPosition(m_CurrentPosition.x, m_CurrentPosition.y);
    }
}

}  // namespace Mario
