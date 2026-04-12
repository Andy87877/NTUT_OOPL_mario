/**
 * @file AudioType.hpp
 * @brief Enum classes and path mappings for Background Music (BGM) and Sound
 * Effects (SFX).
 * @inheritance None
 */
#ifndef MARIO_AUDIO_TYPE_HPP
#define MARIO_AUDIO_TYPE_HPP

#include <string>
#include <vector>

namespace Mario {

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
    "Resources/Audio/BGM/18. Saved the Princess (NES) [VS. Super Mario "
    "Bros.].wav",
    "Resources/Audio/BGM/19. Name Entry (VS. Super Mario Bros.).wav",
    "Resources/Audio/BGM/20. Saved the Princess (NES) [Super Mario Bros. 2] "
    "(Super Mario Bros. 2 Japan).wav",
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

}  // namespace Mario

#endif  // MARIO_AUDIO_TYPE_HPP
