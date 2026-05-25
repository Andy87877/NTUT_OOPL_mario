/**
 * @file PlayerForm.hpp
 * @brief Strategy/State pattern interface representing Mario's power-up form.
 *        Encapsulates form-specific attributes, dimensions, transitions, and sprite state/star state calculations.
 * @inheritance IPlayerForm <- {SmallPlayerForm, BigPlayerForm, FirePlayerForm, SmallStarPlayerForm, BigStarPlayerForm}
 */

#ifndef MARIO_PLAYER_FORM_HPP
#define MARIO_PLAYER_FORM_HPP

#include <memory>
#include <string>
#include "Mario/Player/PlayerState.hpp"

namespace Mario {

/**
 * @class IPlayerForm
 * @brief Strategy/State pattern interface representing Mario's power-up form.
 *        Encapsulates form-specific characteristics (dimensions, abilities, and form transitions)
 *        to eliminate monolithic switch-case/conditional statements from PlayerState.
 * @inheritance None (base interface class)
 */
class IPlayerForm {
   public:
    virtual ~IPlayerForm() = default;

    virtual PowerState GetPowerState() const = 0;
    virtual int GetHeight(bool crouching) const = 0;
    virtual bool IsBigOrFire() const = 0;
    virtual bool CanShootFire() const = 0;
    virtual bool IsStar() const = 0;
    virtual int GetSpriteState(bool isFireShooting) const = 0;
    virtual int GetStarState(int starTimer, bool isFireShooting) const = 0;
    virtual std::string GetFormName() const = 0;

    /**
     * Handles taking damage from the perspective of this form.
     * @return The next form after damage is resolved, or nullptr if this was a lethal hit.
     */
    virtual std::unique_ptr<IPlayerForm> TakeDamage() const = 0;

    /**
     * Handles power-up state transition.
     * @param newState The state we are attempting to transition to.
     * @return The new form after power-up.
     */
    virtual std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const = 0;
};

// Factory function to create the initial form based on the state ID or enum
std::unique_ptr<IPlayerForm> CreatePlayerForm(PowerState state, PowerState memoryState = PowerState::SMALL);

/**
 * @class SmallPlayerForm
 * @brief Class representing Small Mario form.
 * @inheritance IPlayerForm -> SmallPlayerForm
 */
class SmallPlayerForm : public IPlayerForm {
   public:
    PowerState GetPowerState() const override;
    int GetHeight(bool crouching) const override;
    bool IsBigOrFire() const override;
    bool CanShootFire() const override;
    bool IsStar() const override;
    int GetSpriteState(bool isFireShooting) const override;
    int GetStarState(int starTimer, bool isFireShooting) const override;
    std::string GetFormName() const override;

    std::unique_ptr<IPlayerForm> TakeDamage() const override;
    std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const override;
};

/**
 * @class BigPlayerForm
 * @brief Class representing Big Mario form.
 * @inheritance IPlayerForm -> BigPlayerForm
 */
class BigPlayerForm : public IPlayerForm {
   public:
    PowerState GetPowerState() const override;
    int GetHeight(bool crouching) const override;
    bool IsBigOrFire() const override;
    bool CanShootFire() const override;
    bool IsStar() const override;
    int GetSpriteState(bool isFireShooting) const override;
    int GetStarState(int starTimer, bool isFireShooting) const override;
    std::string GetFormName() const override;

    std::unique_ptr<IPlayerForm> TakeDamage() const override;
    std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const override;
};

/**
 * @class FirePlayerForm
 * @brief Class representing Fire Mario form.
 * @inheritance IPlayerForm -> FirePlayerForm
 */
class FirePlayerForm : public IPlayerForm {
   public:
    PowerState GetPowerState() const override;
    int GetHeight(bool crouching) const override;
    bool IsBigOrFire() const override;
    bool CanShootFire() const override;
    bool IsStar() const override;
    int GetSpriteState(bool isFireShooting) const override;
    int GetStarState(int starTimer, bool isFireShooting) const override;
    std::string GetFormName() const override;

    std::unique_ptr<IPlayerForm> TakeDamage() const override;
    std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const override;
};

/**
 * @class SmallStarPlayerForm
 * @brief Class representing Small Mario form with Star invincibility active.
 * @inheritance IPlayerForm -> SmallStarPlayerForm
 */
class SmallStarPlayerForm : public IPlayerForm {
   public:
    explicit SmallStarPlayerForm(PowerState memoryState);
    PowerState GetPowerState() const override;
    int GetHeight(bool crouching) const override;
    bool IsBigOrFire() const override;
    bool CanShootFire() const override;
    bool IsStar() const override;
    int GetSpriteState(bool isFireShooting) const override;
    int GetStarState(int starTimer, bool isFireShooting) const override;
    std::string GetFormName() const override;

    std::unique_ptr<IPlayerForm> TakeDamage() const override;
    std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const override;

    PowerState GetMemoryState() const { return m_MemoryState; }

   private:
    PowerState m_MemoryState;
};

/**
 * @class BigStarPlayerForm
 * @brief Class representing Big Mario form with Star invincibility active.
 * @inheritance IPlayerForm -> BigStarPlayerForm
 */
class BigStarPlayerForm : public IPlayerForm {
   public:
    explicit BigStarPlayerForm(PowerState memoryState);
    PowerState GetPowerState() const override;
    int GetHeight(bool crouching) const override;
    bool IsBigOrFire() const override;
    bool CanShootFire() const override;
    bool IsStar() const override;
    int GetSpriteState(bool isFireShooting) const override;
    int GetStarState(int starTimer, bool isFireShooting) const override;
    std::string GetFormName() const override;

    std::unique_ptr<IPlayerForm> TakeDamage() const override;
    std::unique_ptr<IPlayerForm> PowerUp(PowerState newState) const override;

    PowerState GetMemoryState() const { return m_MemoryState; }

   private:
    PowerState m_MemoryState;
};

}  // namespace Mario

#endif  // MARIO_PLAYER_FORM_HPP
