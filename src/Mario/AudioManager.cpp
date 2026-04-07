/**
 * @file AudioManager.cpp
 * @brief Implementation of audio manager.
 * @inheritance IAudioService <- AudioManager
 */
#include "Mario/AudioManager.hpp"

#include "Util/Logger.hpp"

namespace Mario {

AudioManager::AudioManager() : m_Volume(1.0f), m_Muted(false) {
    LOG_INFO("AudioManager initialized");
}

void AudioManager::PlayBGM(const std::string& name) {
    if (m_Muted) return;

    if (m_CurrentBGM != name) {
        LOG_DEBUG("Playing BGM: {}", name);
        m_CurrentBGM = name;
        // Actual audio playback would go here
        // Could use PTSD's Util::Audio or external library
    }
}

void AudioManager::StopBGM() {
    LOG_DEBUG("Stopping BGM");
    m_CurrentBGM.clear();
    // Stop playback
}

void AudioManager::PlaySFX(const std::string& name) {
    if (m_Muted) return;

    LOG_DEBUG("Playing SFX: {}", name);
    // Actual SFX playback would go here
}

void AudioManager::SetVolume(float volume) {
    m_Volume = volume;
    LOG_DEBUG("Audio volume set to: {}", volume);
}

void AudioManager::SetMuted(bool muted) {
    m_Muted = muted;
    LOG_DEBUG("Audio muted: {}", muted ? "true" : "false");
}

}  // namespace Mario
