/**
 * @file UIWidgets.hpp
 * @brief Lightweight PTSD-wrapper UI components used by UIManager / CoinUI /
 * FloatingText. UIImage and UIText are merged here because they are always used
 * together and have no consumers outside the HUD sub-system.
 * @inheritance Util::GameObject <- UIImage
 *              Util::GameObject <- UIText
 */
#ifndef MARIO_UI_WIDGETS_HPP
#define MARIO_UI_WIDGETS_HPP

#include <memory>
#include <string>

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Text.hpp"

namespace Mario {

// ============================================================================
// UIImage — wraps Util::Image as a positioned GameObject for the Renderer
// ============================================================================
/**
 * Wraps PTSD's Util::Image inside a GameObject so it can be added to the
 * Renderer and positioned as a UI element.
 * @inheritance Util::GameObject <- UIImage
 */
class UIImage : public Util::GameObject {
   public:
    explicit UIImage(const std::string& imagePath) {
        m_Image = std::make_shared<Util::Image>(imagePath);
        SetDrawable(m_Image);
        SetZIndex(100.0f);  // UI always rendered on top of game objects
    }

    ~UIImage() override = default;

    void SetPosition(float x, float y) { m_Transform.translation = {x, y}; }

    void SetImagePath(const std::string& imagePath) {
        m_Image = std::make_shared<Util::Image>(imagePath);
        SetDrawable(m_Image);
    }

   private:
    std::shared_ptr<Util::Image> m_Image;
};

// ============================================================================
// UIText — wraps Util::Text as a positioned GameObject for the Renderer
// ============================================================================
/**
 * Wraps PTSD's Util::Text inside a GameObject so it can be added to the
 * Renderer and positioned as a UI element.
 * @inheritance Util::GameObject <- UIText
 */
class UIText : public Util::GameObject {
   public:
    UIText(const std::string& fontPath, int size, const std::string& text,
           const Util::Color& color) {
        std::string safeText = text.empty() ? " " : text;
        m_Text = std::make_shared<Util::Text>(fontPath, size, safeText, color);
        SetDrawable(m_Text);
        SetZIndex(100.0f);  // UI always rendered on top of game objects
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

#endif  // MARIO_UI_WIDGETS_HPP
