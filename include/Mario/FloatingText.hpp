/**
 * @file FloatingText.hpp
 * @brief HUD floating text effect for score numbers.
 * @inheritance None (Wrapper data class)
 */
#ifndef MARIO_FLOATING_TEXT_HPP
#define MARIO_FLOATING_TEXT_HPP

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Mario/UIWidgets.hpp"

namespace Mario {

class FloatingText {
   public:
    FloatingText(float worldX, float worldY, const std::string& text,
                 int durationFrames = 60);
    ~FloatingText() = default;

    void Update();

    bool IsExpired() const { return m_LifetimeCounter <= 0; }

    std::shared_ptr<UIText> GetUIText() const { return m_UIText; }

   private:
    std::shared_ptr<UIText> m_UIText;
    glm::vec2 m_CurrentPosition{0.0f, 0.0f};
    int m_LifetimeCounter;
    int m_OriginalDuration;
};

}  // namespace Mario

#endif  // MARIO_FLOATING_TEXT_HPP
