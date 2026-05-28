/**
 * @file AxeSequenceSceneHandler.cpp
 * @brief 8-4 boss-defeat ending cutscene handler (App::State::AXE_SEQUENCE).
 *        Handles bridge collapse, Bowser falling, and walking to Princess.
 * @inheritance ISceneHandler <- AxeSequenceSceneHandler
 */
#include "Mario/Scenes/AxeSequenceSceneHandler.hpp"

#include <vector>

#include "App.hpp"
#include "Mario/Core/GameConfig.hpp"
#include "Mario/Level/Block.hpp"
#include "Mario/Services/AudioManager.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void AxeSequenceSceneHandler::OnEnter(App& app) {
    m_Phase = Phase::START;
    m_tick_count = 0;
    m_bowser = nullptr;
    m_princess = nullptr;

    // Find Bowser and Princess in the entity list using polymorphic queries
    // — avoids EntityType comparisons outside the Factory (OCP / DIP).
    for (const auto& entity : app.GetEntities()) {
        auto* b = entity ? entity->GetBehavior() : nullptr;
        if (b && b->IsBowser()) m_bowser = entity;
        if (b && b->IsPrincess()) m_princess = entity;
    }

    auto& player = app.GetPlayer();
    if (player) {
        player->GetState().SetControllable(false);
        player->GetState().SetVelX(0.0f);
        player->GetState().SetVelY(0.0);
        player->GetState().SetMovingRight(false);
        player->GetState().SetMovingLeft(false);
        player->GetState().SetPoleSliding(false);
        player->SetVisible(true);
    }

    LOG_INFO("8-4 Axe sequence started.");
}

void AxeSequenceSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    m_tick_count++;

    // Keep player grounded/animated during the cutscene so WALK_TO_PRINCESS
    // looks like walking instead of horizontal gliding.
    auto& ps = player->GetState();
    float yDelta = ps.ApplyGravity();
    ps.SetY(ps.GetY() + yDelta);
    std::vector<Mario::Level::SpawnPoint> unusedSpawns;
    app.GetCollisionManager().CheckPlayerBlockCollision(
        *player, *level, app.GetCamera(), app.GetGameState(),
        app.GetUIManager(), &unusedSpawns);
    ps.Tick();

    UpdateAxeSequence(*player, *level);

    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels(),
                           app.GetCurrentLevelName(), true);
    level->UpdateBlocks(app.GetCamera().GetOffset());

    for (auto& entity : app.GetEntities()) {
        if (entity->GetState().IsActive()) {
            entity->GetState().Tick();
            entity->UpdateView(app.GetCamera().GetOffset());
        }
    }

    player->UpdateView(app.GetCamera().GetOffset());

    if (m_Phase == Phase::COMPLETED) {
        app.GetGameState().SetGameWon(true);
        app.AdvanceToNextLevel();
    }
}

void AxeSequenceSceneHandler::OnRender(App& app) {
    app.ApplyBackground(true);  // Black background for ending cutscene
    app.GetRenderer().Update();
    auto endPhase =
        (m_Phase == Phase::PRINCESS_DIALOG || m_Phase == Phase::COMPLETED)
            ? Mario::UIManager::EndingTextPhase::CREDITS
            : Mario::UIManager::EndingTextPhase::NONE;
    app.GetUIManager().SetEndingPhase(endPhase);
    app.GetUIManager().Update(Mario::UIManager::State::AXE_SEQUENCE);
}

void AxeSequenceSceneHandler::UpdateAxeSequence(Player& player, Level& level) {
    switch (m_Phase) {
        case Phase::START:
            if (m_tick_count > 30) {  // ~0.5s pause
                m_Phase = Phase::BRIDGE_COLLAPSE;
                m_tick_count = 0;
                LOG_INFO("8-4: Collapsing bridge.");

                // Make bridge blocks fall
                level.CollapseBridge();
            }
            break;

        case Phase::BRIDGE_COLLAPSE:
            if (m_tick_count > 60) {  // ~1s
                m_Phase = Phase::BOWSER_FALL;
                m_tick_count = 0;
                if (m_bowser && m_bowser->GetState().IsActive()) {
                    LOG_INFO("8-4: Bowser falls.");
                    auto& bowserState = m_bowser->GetState();
                    m_bowser->SetBehavior(nullptr);  // Disable AI
                    bowserState.SetCollidable(false);
                    bowserState.SetGravity(true);
                    bowserState.SetVelY(-5.0f);  // Give a little push
                }
            }
            break;

        case Phase::BOWSER_FALL:
            if (m_tick_count > 180) {  // ~3s
                m_Phase = Phase::WALK_TO_PRINCESS;
                m_tick_count = 0;
                LOG_INFO("8-4: Walking to Princess.");
            }
            break;

        case Phase::WALK_TO_PRINCESS: {
            PlayerState& ps = player.GetState();
            ps.SetMovingRight(true);
            ps.SetFacingRight(true);
            ps.SetX(ps.GetX() +
                    GameConfig::SCALED_SPEED * 0.5f);  // Walk slower

            if (m_princess && m_princess->GetState().IsActive()) {
                float princessX = m_princess->GetState().GetX();
                if (ps.GetX() >= princessX - ps.GetWidth()) {
                    ps.SetMovingRight(false);
                    ps.SetVelX(0);
                    m_Phase = Phase::PRINCESS_DIALOG;
                    m_tick_count = 0;
                    LOG_INFO("8-4: Reached Princess.");
                }
            } else {
                if (m_tick_count > 300) {  // Walk for ~5s fallback
                    m_Phase = Phase::PRINCESS_DIALOG;
                }
            }
            break;
        }

        case Phase::PRINCESS_DIALOG:
            if (m_tick_count > 300) {  // ~5s
                m_Phase = Phase::COMPLETED;
                LOG_INFO("8-4: Game complete!");
            }
            break;

        default:
            break;
    }
}

}  // namespace Mario
