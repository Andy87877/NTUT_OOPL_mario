/**
 * @file FloatingText.cpp
 * @brief Logic for floating text effect with fade-out and scale animation.
 * @inheritance None (Wrapper with composition)
 */
#include "Mario/UI/FloatingText.hpp"

#include "Util/Color.hpp"
#include "Util/Logger.hpp"

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

        // Fade out: decrease alpha as time expires
        float progress =
            static_cast<float>(m_LifetimeCounter) / m_OriginalDuration;
        int alpha = static_cast<int>(255.0f * progress);
        alpha = (alpha < 0) ? 0 : alpha;
        m_UIText->SetTextColor(
            Util::Color(255, 255, 255, static_cast<Uint8>(alpha)));
        // If UIText supports scale, apply it here
        // m_UIText->SetScale(scale);
    }
}

}  // namespace Mario
