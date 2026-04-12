/**
 * @file FloatingText.cpp
 * @brief Logic for floating text effect.
 * @inheritance None (Wrapper with composition)
 */
#include "Mario/FloatingText.hpp"

#include "Util/Color.hpp"

namespace Mario {

FloatingText::FloatingText(float ptsdX, float ptsdY, const std::string& text,
                           int durationFrames)
    : m_LifetimeCounter(durationFrames), m_OriginalDuration(durationFrames) {
    // Initialize position in PTSD coordinate system
    m_CurrentPosition = {ptsdX, ptsdY};
    m_UIText =
        std::make_shared<UIText>(std::string(RESOURCE_DIR) + "/Font/mario.ttf",
                                 16, text, Util::Color::FromRGB(255, 255, 255));
    m_UIText->SetPosition(ptsdX, ptsdY);
}

void FloatingText::Update() {
    m_LifetimeCounter--;

    if (m_LifetimeCounter > 0) {
        // Float upward in PTSD coordinate space (positive Y is up)
        m_CurrentPosition.y -= 1.0f;  // Decrease Y to move up in PTSD coords
        m_UIText->SetPosition(m_CurrentPosition.x, m_CurrentPosition.y);
    }
}

}  // namespace Mario
