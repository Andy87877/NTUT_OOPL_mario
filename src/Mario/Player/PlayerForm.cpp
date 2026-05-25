/**
 * @file PlayerForm.cpp
 * @brief Strategy/State pattern implementations for Mario's power-up forms.
 *        Implements dimensions, abilities, damage resolution, and power-up transitions for each form.
 * @inheritance IPlayerForm <- {SmallPlayerForm, BigPlayerForm, FirePlayerForm, SmallStarPlayerForm, BigStarPlayerForm}
 */

#include "Mario/Player/PlayerForm.hpp"
#include "Mario/Player/PlayerState.hpp"
#include "Mario/Core/GameConfig.hpp"

namespace Mario {

std::unique_ptr<IPlayerForm> CreatePlayerForm(PowerState state, PowerState memoryState) {
    switch (state) {
        case PowerState::SMALL:
            return std::make_unique<SmallPlayerForm>();
        case PowerState::BIG:
            return std::make_unique<BigPlayerForm>();
        case PowerState::FIRE:
            return std::make_unique<FirePlayerForm>();
        case PowerState::SMALL_STAR:
            return std::make_unique<SmallStarPlayerForm>(memoryState);
        case PowerState::BIG_STAR:
            return std::make_unique<BigStarPlayerForm>(memoryState);
        default:
            return std::make_unique<SmallPlayerForm>();
    }
}

// ============================================================================
// SmallPlayerForm
// ============================================================================

PowerState SmallPlayerForm::GetPowerState() const {
    return PowerState::SMALL;
}

int SmallPlayerForm::GetHeight(bool /*crouching*/) const {
    return GameConfig::TILE_SIZE;
}

bool SmallPlayerForm::IsBigOrFire() const {
    return false;
}

bool SmallPlayerForm::CanShootFire() const {
    return false;
}

bool SmallPlayerForm::IsStar() const {
    return false;
}

int SmallPlayerForm::GetSpriteState(bool /*isFireShooting*/) const {
    return static_cast<int>(PowerState::SMALL);
}

int SmallPlayerForm::GetStarState(int /*starTimer*/, bool /*isFireShooting*/) const {
    return 0;
}

std::string SmallPlayerForm::GetFormName() const {
    return "Small";
}

std::unique_ptr<IPlayerForm> SmallPlayerForm::TakeDamage() const {
    return nullptr; // Lethal hit
}

std::unique_ptr<IPlayerForm> SmallPlayerForm::PowerUp(PowerState newState) const {
    switch (newState) {
        case PowerState::BIG:
            return std::make_unique<BigPlayerForm>();
        case PowerState::FIRE:
            return std::make_unique<FirePlayerForm>();
        case PowerState::SMALL_STAR:
            return std::make_unique<SmallStarPlayerForm>(PowerState::SMALL);
        case PowerState::BIG_STAR:
            return std::make_unique<BigStarPlayerForm>(PowerState::BIG);
        default:
            return std::make_unique<SmallPlayerForm>();
    }
}

// ============================================================================
// BigPlayerForm
// ============================================================================

PowerState BigPlayerForm::GetPowerState() const {
    return PowerState::BIG;
}

int BigPlayerForm::GetHeight(bool crouching) const {
    return crouching ? GameConfig::TILE_SIZE : GameConfig::TILE_SIZE * 2;
}

bool BigPlayerForm::IsBigOrFire() const {
    return true;
}

bool BigPlayerForm::CanShootFire() const {
    return false;
}

bool BigPlayerForm::IsStar() const {
    return false;
}

int BigPlayerForm::GetSpriteState(bool /*isFireShooting*/) const {
    return static_cast<int>(PowerState::BIG);
}

int BigPlayerForm::GetStarState(int /*starTimer*/, bool /*isFireShooting*/) const {
    return 0;
}

std::string BigPlayerForm::GetFormName() const {
    return "Big";
}

std::unique_ptr<IPlayerForm> BigPlayerForm::TakeDamage() const {
    return std::make_unique<SmallPlayerForm>(); // Shrinks to Small
}

std::unique_ptr<IPlayerForm> BigPlayerForm::PowerUp(PowerState newState) const {
    switch (newState) {
        case PowerState::FIRE:
            return std::make_unique<FirePlayerForm>();
        case PowerState::SMALL_STAR:
        case PowerState::BIG_STAR:
            return std::make_unique<BigStarPlayerForm>(PowerState::BIG);
        default:
            return std::make_unique<BigPlayerForm>();
    }
}

// ============================================================================
// FirePlayerForm
// ============================================================================

PowerState FirePlayerForm::GetPowerState() const {
    return PowerState::FIRE;
}

int FirePlayerForm::GetHeight(bool crouching) const {
    return crouching ? GameConfig::TILE_SIZE : GameConfig::TILE_SIZE * 2;
}

bool FirePlayerForm::IsBigOrFire() const {
    return true;
}

bool FirePlayerForm::CanShootFire() const {
    return true;
}

bool FirePlayerForm::IsStar() const {
    return false;
}

int FirePlayerForm::GetSpriteState(bool /*isFireShooting*/) const {
    return static_cast<int>(PowerState::FIRE);
}

int FirePlayerForm::GetStarState(int /*starTimer*/, bool /*isFireShooting*/) const {
    return 0;
}

std::string FirePlayerForm::GetFormName() const {
    return "Fire";
}

std::unique_ptr<IPlayerForm> FirePlayerForm::TakeDamage() const {
    return std::make_unique<BigPlayerForm>(); // Shrinks to Big
}

std::unique_ptr<IPlayerForm> FirePlayerForm::PowerUp(PowerState newState) const {
    switch (newState) {
        case PowerState::SMALL_STAR:
        case PowerState::BIG_STAR:
            return std::make_unique<BigStarPlayerForm>(PowerState::FIRE);
        default:
            return std::make_unique<FirePlayerForm>();
    }
}

// ============================================================================
// SmallStarPlayerForm
// ============================================================================

SmallStarPlayerForm::SmallStarPlayerForm(PowerState memoryState)
    : m_MemoryState(memoryState) {}

PowerState SmallStarPlayerForm::GetPowerState() const {
    return PowerState::SMALL_STAR;
}

int SmallStarPlayerForm::GetHeight(bool /*crouching*/) const {
    return GameConfig::TILE_SIZE;
}

bool SmallStarPlayerForm::IsBigOrFire() const {
    return false;
}

bool SmallStarPlayerForm::CanShootFire() const {
    return m_MemoryState == PowerState::FIRE;
}

bool SmallStarPlayerForm::IsStar() const {
    return true;
}

int SmallStarPlayerForm::GetSpriteState(bool isFireShooting) const {
    if (isFireShooting && m_MemoryState == PowerState::FIRE) {
        return static_cast<int>(PowerState::FIRE);
    }
    return static_cast<int>(PowerState::SMALL_STAR);
}

int SmallStarPlayerForm::GetStarState(int starTimer, bool isFireShooting) const {
    if (isFireShooting && m_MemoryState == PowerState::FIRE) {
        return 0;
    }
    return (starTimer / 10) % 4;
}

std::string SmallStarPlayerForm::GetFormName() const {
    return "SmallStar";
}

std::unique_ptr<IPlayerForm> SmallStarPlayerForm::TakeDamage() const {
    return std::make_unique<SmallStarPlayerForm>(m_MemoryState); // Invincible
}

std::unique_ptr<IPlayerForm> SmallStarPlayerForm::PowerUp(PowerState newState) const {
    if (newState == PowerState::BIG || newState == PowerState::BIG_STAR) {
        return std::make_unique<BigStarPlayerForm>(PowerState::BIG);
    }
    if (newState == PowerState::FIRE) {
        return std::make_unique<BigStarPlayerForm>(PowerState::FIRE);
    }
    return std::make_unique<SmallStarPlayerForm>(m_MemoryState);
}

// ============================================================================
// BigStarPlayerForm
// ============================================================================

BigStarPlayerForm::BigStarPlayerForm(PowerState memoryState)
    : m_MemoryState(memoryState) {}

PowerState BigStarPlayerForm::GetPowerState() const {
    return PowerState::BIG_STAR;
}

int BigStarPlayerForm::GetHeight(bool crouching) const {
    return crouching ? GameConfig::TILE_SIZE : GameConfig::TILE_SIZE * 2;
}

bool BigStarPlayerForm::IsBigOrFire() const {
    return true;
}

bool BigStarPlayerForm::CanShootFire() const {
    return m_MemoryState == PowerState::FIRE;
}

bool BigStarPlayerForm::IsStar() const {
    return true;
}

int BigStarPlayerForm::GetSpriteState(bool isFireShooting) const {
    if (isFireShooting && m_MemoryState == PowerState::FIRE) {
        return static_cast<int>(PowerState::FIRE);
    }
    return static_cast<int>(PowerState::BIG_STAR);
}

int BigStarPlayerForm::GetStarState(int starTimer, bool isFireShooting) const {
    if (isFireShooting && m_MemoryState == PowerState::FIRE) {
        return 0;
    }
    return (starTimer / 10) % 4;
}

std::string BigStarPlayerForm::GetFormName() const {
    return "BigStar";
}

std::unique_ptr<IPlayerForm> BigStarPlayerForm::TakeDamage() const {
    return std::make_unique<BigStarPlayerForm>(m_MemoryState); // Invincible
}

std::unique_ptr<IPlayerForm> BigStarPlayerForm::PowerUp(PowerState newState) const {
    if (newState == PowerState::FIRE) {
        return std::make_unique<BigStarPlayerForm>(PowerState::FIRE);
    }
    return std::make_unique<BigStarPlayerForm>(m_MemoryState);
}

}  // namespace Mario
