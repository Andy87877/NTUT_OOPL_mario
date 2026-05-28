/**
 * @file IAudioService.hpp
 * @brief Master abstract interface for the audio service (BGM, SFX), following
 *        Dependency Inversion Principle (DIP).
 *        Allows swapping implementations (e.g. mock for testing).
 * @inheritance None (interface)
 */
#ifndef MARIO_I_AUDIO_SERVICE_HPP
#define MARIO_I_AUDIO_SERVICE_HPP

#include <string>

#include "Mario/Core/GameConfig.hpp"
#include "Mario/Services/AudioType.hpp"

namespace Mario {

/**
 * Abstract interface for audio playback.
 */
class IAudioService {
   public:
    virtual ~IAudioService() = default;

    /**
     * Play background music (loops).
     * @param name Audio track name/ID
     */
    virtual void PlayBGM(BGMName name) = 0;

    /**
     * Stop background music.
     */
    virtual void StopBGM() = 0;

    /**
     * Play sound effect (one-shot).
     * @param name Sound effect name/ID
     */
    virtual void PlaySFX(SFXName name) = 0;

    /**
     * Set volume (0.0 = mute, 1.0 = full).
     */
    virtual void SetVolume(float volume) = 0;

    /**
     * Mute/unmute all audio.
     */
    virtual void SetMuted(bool muted) = 0;

    /**
     * Select and play the correct BGM track for a level.
     * Accepts a BGMTheme enum so no caller ever compares level-name strings
     * against BGMName values (OCP: adding a new theme requires only a new
     * enum value, not a grep across the codebase).
     * @param theme          BGMTheme returned by Level::GetBGMTheme().
     * @param timeRemaining  Remaining level time; <= 100 triggers hurry-up.
     */
    virtual void PlayBGMForLevel(BGMTheme theme, int timeRemaining) = 0;
};

}  // namespace Mario

#endif  // MARIO_I_AUDIO_SERVICE_HPP
