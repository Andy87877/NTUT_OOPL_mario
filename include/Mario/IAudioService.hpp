/**
 * @file IAudioService.hpp
 * @brief Interface for audio services (BGM, SFX) following Dependency
 * Injection. Allows swapping implementations for testing or real audio.
 * @inheritance None (interface)
 */
#ifndef MARIO_I_AUDIO_SERVICE_HPP
#define MARIO_I_AUDIO_SERVICE_HPP

#include <string>

namespace Mario {

/**
 * Abstract interface for audio playback.
 * Can be implemented with real audio library or mock for testing.
 */
class IAudioService {
   public:
    virtual ~IAudioService() = default;

    /**
     * Play background music (loops).
     * @param name Audio track name/ID
     */
    virtual void PlayBGM(const std::string& name) = 0;

    /**
     * Stop background music.
     */
    virtual void StopBGM() = 0;

    /**
     * Play sound effect (one-shot).
     * @param name Sound effect name/ID
     */
    virtual void PlaySFX(const std::string& name) = 0;

    /**
     * Set volume (0.0 = mute, 1.0 = full).
     */
    virtual void SetVolume(float volume) = 0;

    /**
     * Mute/unmute all audio.
     */
    virtual void SetMuted(bool muted) = 0;
};

}  // namespace Mario

#endif  // MARIO_I_AUDIO_SERVICE_HPP
