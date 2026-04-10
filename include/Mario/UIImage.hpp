/**
 * @file UIImage.hpp
 * @brief UI Image component wrapping Util::Image as a GameObject.
 * @inheritance Util::GameObject
 */
#ifndef MARIO_UI_IMAGE_HPP
#define MARIO_UI_IMAGE_HPP

#include <memory>
#include <string>

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"

namespace Mario {

/**
 * Wraps PTSD's Util::Image inside a GameObject so it can be added to the
 * Renderer and positioned like UI elements.
 */
class UIImage : public Util::GameObject {
   public:
    explicit UIImage(const std::string& imagePath) {
        m_Image = std::make_shared<Util::Image>(imagePath);
        SetDrawable(m_Image);
        SetZIndex(100.0f);  // Make sure UI is drawn on top
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

}  // namespace Mario

#endif  // MARIO_UI_IMAGE_HPP
