/**
 * @file CoinUI.hpp
 * @brief Coin display UI component with animated coin icon.
 *        Displays both the animated coin sprite (CoinTop1~3) and coin count.
 * @inheritance Util::GameObject (composite)
 */
#ifndef MARIO_COIN_UI_HPP
#define MARIO_COIN_UI_HPP

#include <memory>
#include <string>

#include "Mario/UIWidgets.hpp"

namespace Mario {

/**
 * Composite UI element for coin display.
 * Manages:
 *   - Animated coin icon (cycles through CoinTop1.png, CoinTop2.png,
 * CoinTop3.png)
 *   - Text showing current coin count (e.g., "x00", "x99")
 *
 * Layout:
 *   [CoinIcon] x[Count]
 *
 * The coin icon animates every FRAME_INTERVAL frames.
 */
class CoinUI {
   public:
    /**
     * Initialize coin UI at specified position.
     * @param fontPath Path to font file
     * @param fontSize Font size for text
     * @param screenX Screen X position in pixels (left-to-right: 0-1280)
     * @param screenY Screen Y position in pixels (top-to-bottom: 0-720)
     * @param coinScale Scale multiplier for coin image (default 2.0f for 2x
     * size)
     */
    CoinUI(const std::string& fontPath, int fontSize, float screenX,
           float screenY, float coinScale = 4.0f);

    ~CoinUI() = default;

    /**
     * Update coin animation and text content.
     * @param coinCount Current coin count to display (0-99)
     */
    void Update(int coinCount);

    /**
     * Get the animated coin icon for rendering.
     */
    std::shared_ptr<UIImage> GetCoinImage() const { return m_CoinImage; }

    /**
     * Get the coin count text for rendering.
     */
    std::shared_ptr<UIText> GetCountText() const { return m_CountText; }

   private:
    // Animation resources
    static constexpr int FRAME_INTERVAL = 10;  // Change sprite every 10 frames
    static constexpr const char* COIN_SPRITE_BASE =
        "/Sprites/CoinTop";  // CoinTop1.png, CoinTop2.png, CoinTop3.png
    static constexpr int COIN_FRAME_COUNT = 3;  // 3 animation frames

    int m_AnimationCounter = 0;  // Increments to cycle animation
    int m_CurrentFrame = 0;      // Current frame (0, 1, 2)
    float m_CoinScale = 4.0f;    // Scale multiplier for coin image

    // UI Components
    std::shared_ptr<UIImage> m_CoinImage;
    std::shared_ptr<UIText> m_CountText;

    // Layout offsets (relative to screen position)
    float m_ScreenX;
    float m_ScreenY;

    /**
     * Helper: Convert screen coordinates to PTSD world coordinates.
     * Screen: (0-1280, 0-720)
     * PTSD:   (center at 0,0, left=-640, right=640, top=360, bottom=-360)
     */
    float ScreenXToPTSD(float screenX) const;
    float ScreenYToPTSD(float screenY) const;

    /**
     * Helper: Update the coin sprite based on current frame.
     */
    void UpdateCoinSprite();
};

}  // namespace Mario

#endif  // MARIO_COIN_UI_HPP
