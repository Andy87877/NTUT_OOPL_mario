/**
 * @file AudioManager.hpp
 * @brief Audio management service for BGM and SFX playback.
 * @inheritance IAudioService
 */
#ifndef MARIO_AUDIO_MANAGER_HPP
#define MARIO_AUDIO_MANAGER_HPP

#include <memory>
#include <string>

#include "Mario/IAudioService.hpp"

namespace Mario {

/**
 * Concrete implementation of audio service.
 * Handles BGM (background music) and SFX (sound effects) playback.
 * For now, provides stub implementation; can integrate with
 * PTSD Util::Audio or other audio library.
 */
class AudioManager : public IAudioService {
   public:
    AudioManager();
    virtual ~AudioManager() = default;

    void PlayBGM(const std::string& name) override;
    void StopBGM() override;
    void PlaySFX(const std::string& name) override;
    void SetVolume(float volume) override;
    void SetMuted(bool muted) override;

   private:
    float m_Volume = 1.0f;
    bool m_Muted = false;
    std::string m_CurrentBGM;
};

}  // namespace Mario

#endif  // MARIO_AUDIO_MANAGER_HPP
