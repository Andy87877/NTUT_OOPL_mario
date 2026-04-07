/**
 * @file UIText.hpp
 * @brief UI Text component wrapping Util::Text as a GameObject.
 * @inheritance Util::GameObject
 */
#ifndef MARIO_UI_TEXT_HPP
#define MARIO_UI_TEXT_HPP

#include <memory>
#include <string>

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"

namespace Mario {

/**
 * Wraps PTSD's Util::Text inside a GameObject so it can be added to the
 * Renderer.
 */
class UIText : public Util::GameObject {
   public:
    UIText(const std::string& fontPath, int size, const std::string& text,
           const Util::Color& color) {
        std::string safeText = text.empty() ? " " : text;
        m_Text = std::make_shared<Util::Text>(fontPath, size, safeText, color);
        SetDrawable(m_Text);
        SetZIndex(100.0f);  // Make sure UI is drawn on top
    }

    ~UIText() override = default;

    void SetTextContent(const std::string& text) {
        if (m_Text) {
            std::string safeText = text.empty() ? " " : text;
            m_Text->SetText(safeText);
        }
    }

    void SetTextColor(const Util::Color& color) {
        if (m_Text) {
            m_Text->SetColor(color);
        }
    }

    void SetPosition(float x, float y) { m_Transform.translation = {x, y}; }

   private:
    std::shared_ptr<Util::Text> m_Text;
};

}  // namespace Mario

#endif  // MARIO_UI_TEXT_HPP