/**
 * @file AudioManager.hpp
 * @brief Complete audio sub-system: enums, interface, path helper, and
 *        concrete manager — all in one header.
 *
 *        Consolidated from:
 *          AudioType.hpp      — BGMName/SFXName enums and path lists
 *          IAudioService.hpp  — Abstract audio interface (DIP)
 *          AudioPathResolver.hpp — Static path-building helper
 *          AudioManager.hpp   — Singleton concrete implementation
 *
 *        Rationale: all four files are exclusively part of the audio
 *        sub-system and have no independent consumers.
 *
 * @inheritance IAudioService <- AudioManager
 *              None          <- AudioPathResolver (static helper)
 */
#ifndef MARIO_AUDIO_MANAGER_HPP
#define MARIO_AUDIO_MANAGER_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Util/BGM.hpp"
#include "Util/SFX.hpp"

namespace Mario {

// ============================================================================
// BGMName / SFXName enums  (was AudioType.hpp)
// ============================================================================

enum class BGMName {
    GroundTheme,
    UndergroundTheme,
    UnderwaterTheme,
    CastleTheme,
    InvincibilityTheme,
    LevelCompleteTheme,
    CastleCompleteTheme,
    LostALifeTheme,
    GameOverTheme,
    IntoThePipeTheme,
    SavedThePrincessNes,
    GroundThemeHurryUp,
    UndergroundThemeHurryUp,
    UnderwaterThemeHurryUp,
    CastleThemeHurryUp,
    InvincibilityThemeHurryUp,
    IntoThePipeHurryUp,
    SavedThePrincessNesVsSuperMarioBros,
    NameEntryVsSuperMarioBros,
    SavedThePrincessNesSuperMarioBros2SuperMarioBros2Japan,
    GameOverThemeAltUnusedUnused,
};

enum class SFXName {
    _1up,
    Beep,
    BigJump,
    BowserDie,
    Break,
    Bump,
    Coin,
    EnemyFire,
    FireBall,
    Flagpole,
    Item,
    Jump,
    Kick,
    Pause,
    Powerup,
    Skid,
    Squish,
    Thwomp,
    Vine,
    Warp,
};

// clang-format off
const std::vector<std::string> BGMPaths = {
    "Resources/Audio/BGM/01. Ground Theme.wav",
    "Resources/Audio/BGM/02. Underground Theme.wav",
    "Resources/Audio/BGM/03. Underwater Theme.wav",
    "Resources/Audio/BGM/04. Castle Theme.wav",
    "Resources/Audio/BGM/05. Invincibility Theme.wav",
    "Resources/Audio/BGM/06. Level Complete Theme.wav",
    "Resources/Audio/BGM/07. Castle Complete Theme.wav",
    "Resources/Audio/BGM/08. Lost A Life Theme.wav",
    "Resources/Audio/BGM/09. Game Over Theme.wav",
    "Resources/Audio/BGM/10. Into the Pipe Theme.wav",
    "Resources/Audio/BGM/11. Saved the Princess (NES).wav",
    "Resources/Audio/BGM/12. Ground Theme (Hurry Up!).wav",
    "Resources/Audio/BGM/13. Underground Theme (Hurry Up!).wav",
    "Resources/Audio/BGM/14. Underwater Theme (Hurry Up!).wav",
    "Resources/Audio/BGM/15. Castle Theme (Hurry Up!).wav",
    "Resources/Audio/BGM/16. Invincibility Theme (Hurry Up!).wav",
    "Resources/Audio/BGM/17. Into the Pipe (Hurry Up!).wav",
    "Resources/Audio/BGM/18. Saved the Princess (NES) [VS. Super Mario Bros.].wav",
    "Resources/Audio/BGM/19. Name Entry (VS. Super Mario Bros.).wav",
    "Resources/Audio/BGM/20. Saved the Princess (NES) [Super Mario Bros. 2] (Super Mario Bros. 2 Japan).wav",
    "Resources/Audio/BGM/21. Game Over Theme (Alt) [unused] (Unused).wav",
};

const std::vector<std::string> SFXPaths = {
    "Resources/Audio/SFX/01. 1up.wav",
    "Resources/Audio/SFX/02. Beep.wav",
    "Resources/Audio/SFX/03. Big Jump.wav",
    "Resources/Audio/SFX/04. Bowser Die.wav",
    "Resources/Audio/SFX/05. Break.wav",
    "Resources/Audio/SFX/06. Bump.wav",
    "Resources/Audio/SFX/07. Coin.wav",
    "Resources/Audio/SFX/08. Enemy Fire.wav",
    "Resources/Audio/SFX/09. Fire Ball.wav",
    "Resources/Audio/SFX/10. Flagpole.wav",
    "Resources/Audio/SFX/11. Item.wav",
    "Resources/Audio/SFX/12. Jump.wav",
    "Resources/Audio/SFX/13. Kick.wav",
    "Resources/Audio/SFX/14. Pause.wav",
    "Resources/Audio/SFX/15. Powerup.wav",
    "Resources/Audio/SFX/16. Skid.wav",
    "Resources/Audio/SFX/17. Squish.wav",
    "Resources/Audio/SFX/18. Thwomp.wav",
    "Resources/Audio/SFX/19. Vine.wav",
    "Resources/Audio/SFX/20. Warp.wav",
};
// clang-format on

// ============================================================================
// IAudioService  (was IAudioService.hpp)
// Abstract interface for audio playback (Dependency Inversion Principle)
// ============================================================================
/**
 * Abstract audio service interface.
 * AudioManager is the sole implementation; keeping the interface
 * lets unit tests inject a mock without including the full manager.
 * @inheritance None (pure interface)
 */
class IAudioService {
   public:
    virtual ~IAudioService() = default;
    virtual void PlayBGM(BGMName name) = 0;
    virtual void StopBGM() = 0;
    virtual void PlaySFX(SFXName name) = 0;
    virtual void SetVolume(float volume) = 0;
    virtual void SetMuted(bool muted) = 0;

    /**
     * Select and play the correct BGM track for a level.
     * Encapsulates the level-name -> BGMName mapping so callers (App,
     * handlers) never hard-code level names against BGMName values.
     * @param levelName       Current level identifier (e.g. "1-2", "8-4").
     * @param timeRemaining   Remaining level time; <= 100 triggers hurry-up.
     */
    virtual void PlayBGMForLevel(const std::string& levelName,
                                 int timeRemaining) = 0;
};

// ============================================================================
// AudioPathResolver  (was AudioPathResolver.hpp)
// Static helper — builds absolute paths from RESOURCE_DIR.
// Implementation is in AudioManager.cpp.
// ============================================================================
/**
 * Builds full filesystem paths for audio files.
 * @inheritance None (static utility)
 */
class AudioPathResolver {
   public:
    static std::string GetBGMPath(const std::string& filename);
    static std::string GetSFXPath(const std::string& filename);

   private:
    AudioPathResolver() = delete;
    static constexpr const char* BGM_SUBDIR = "/Audio/BGM/";
    static constexpr const char* SFX_SUBDIR = "/Audio/SFX/";
};

// ============================================================================
// AudioManager  — concrete singleton implementation
// ============================================================================
/**
 * Singleton audio service: handles BGM looping and one-shot SFX via PTSD.
 * @inheritance IAudioService <- AudioManager
 */
class AudioManager : public IAudioService {
   public:
    static AudioManager& GetInstance();

    void PlayBGM(BGMName name) override;
    void StopBGM() override;
    void PlaySFX(SFXName name) override;
    void SetVolume(float volume) override;
    void SetMuted(bool muted) override;
    void PlayBGMForLevel(const std::string& levelName,
                         int timeRemaining) override;

   private:
    float m_Volume = 1.0f;
    bool m_Muted = false;
    std::optional<BGMName> m_CurrentBGM;
    bool m_BGMStarted = false;

    std::map<BGMName, std::shared_ptr<Util::BGM>> m_BGMs;
    std::map<SFXName, std::shared_ptr<Util::SFX>> m_SFXs;

    AudioManager();
    ~AudioManager() override = default;
};

}  // namespace Mario

#endif  // MARIO_AUDIO_MANAGER_HPP
