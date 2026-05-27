/**
 * @file LevelManager.hpp
 * @brief Concrete Level Service implementation, encapsulating level asset loading,
 *        entity list coordination, and level completion sequence variables.
 *
 * @inheritance ILevelService <- LevelManager
 */
#ifndef MARIO_LEVEL_MANAGER_HPP
#define MARIO_LEVEL_MANAGER_HPP

#include "Mario/Services/ILevelService.hpp"
#include "Mario/Level/Level.hpp"
#include "Mario/Player/Player.hpp"
#include "Mario/Level/Entity.hpp"
#include "Mario/Core/GameConfig.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Mario {

/**
 * Concrete implementation of ILevelService managing active game states.
 */
class LevelManager : public ILevelService {
   public:
    LevelManager();
    virtual ~LevelManager() override = default;

    // -- ILevelService interface overrides --
    virtual void LoadLevel(App& app, const std::string& levelName) override;
    virtual void StartLevel(App& app) override;
    virtual void PlayCurrentBGM(App& app) override;
    virtual void AddEntityToGame(App& app, std::shared_ptr<Entity> entity) override;
    virtual void AdvanceToNextLevel(App& app) override;

    virtual bool IsUnderground(App& app) const override;
    virtual void ApplyBackground(App& app) override;
    virtual void ApplyBackground(App& app, bool isUnderground) override;

    // -- Subsystem Accessors --
    virtual std::shared_ptr<Level>& GetLevel() override { return m_Level; }
    virtual std::shared_ptr<Player>& GetPlayer() override { return m_Player; }
    virtual std::vector<std::shared_ptr<Entity>>& GetEntities() override { return m_Entities; }
    virtual std::shared_ptr<Entity>& GetFlagEntity() override { return m_FlagEntity; }
    virtual const std::string& GetCurrentLevelName() const override { return m_CurrentLevelName; }

    // -- Primitive mutable accessors for handlers --
    virtual bool& GetLoading() override { return m_Loading; }
    virtual int& GetLoadTimer() override { return m_LoadTimer; }
    virtual int& GetDeathTimer() override { return m_DeathTimer; }
    virtual int& GetESCMenuSelection() override { return m_ESCMenuSelection; }

   private:
    std::shared_ptr<Level> m_Level;
    std::shared_ptr<Player> m_Player;
    std::vector<std::shared_ptr<Entity>> m_Entities;
    std::shared_ptr<Entity> m_FlagEntity;

    std::string m_CurrentLevelName;
    float m_Speed = GameConfig::SCALED_SPEED;

    bool m_Loading = false;
    int m_LoadTimer = -1;
    int m_DeathTimer = -1;
    int m_ESCMenuSelection = 0;
};

}  // namespace Mario

#endif  // MARIO_LEVEL_MANAGER_HPP
