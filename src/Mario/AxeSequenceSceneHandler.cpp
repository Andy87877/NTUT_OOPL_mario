/**
 * @file AxeSequenceSceneHandler.cpp
 * @brief 8-4 boss-defeat ending cutscene handler implementation.
 *        Logic moved from App::UpdateAxeSequence().
 * @inheritance ISceneHandler <- AxeSequenceSceneHandler
 */
#include "Mario/AxeSequenceSceneHandler.hpp"

#include "App.hpp"
#include "Util/Logger.hpp"

namespace Mario {

void AxeSequenceSceneHandler::Update(App& app) {
    auto& player = app.GetPlayer();
    auto& level = app.GetLevel();
    if (!player || !level) return;

    // Find Bowser and Princess in the entity list (cached each frame is fine
    // since the list is small during this cutscene).
    std::shared_ptr<Mario::Entity> bowser = nullptr;
    std::shared_ptr<Mario::Entity> princess = nullptr;
    for (const auto& entity : app.GetEntities()) {
        if (entity->GetState().GetName() == "Bowser") bowser = entity;
        if (entity->GetState().GetName() == "Princess") princess = entity;
    }

    if (!app.GetLevelCompleteCtrl().IsActive()) {
        app.GetLevelCompleteCtrl().StartAxeSequence(*player, bowser, princess);
    }

    bool stillRunning = app.GetLevelCompleteCtrl().Update(
        *player, *level, app.GetCamera().GetOffset());

    app.GetCamera().Update(player->GetWorldX(), level->GetWidthPixels());
    level->UpdateBlocks(app.GetCamera().GetOffset());

    for (auto& entity : app.GetEntities()) {
        if (entity->GetState().IsActive()) {
            entity->GetState().Tick();
            entity->UpdateView(app.GetCamera().GetOffset());
        }
    }

    if (!stillRunning && app.GetLevelCompleteCtrl().IsCompleted()) {
        app.GetGameState().SetGameWon(true);
        app.AdvanceToNextLevel();
    }
}

void AxeSequenceSceneHandler::OnRender(App& app) {
    app.ApplyBackground(true);  // Black background for ending cutscene
    app.GetRenderer().Update();
    auto& lcc = app.GetLevelCompleteCtrl();
    auto endPhase = (lcc.GetPhase() == Mario::EndingPhase::PRINCESS_DIALOG ||
                     lcc.GetPhase() == Mario::EndingPhase::COMPLETED)
                        ? Mario::UIManager::EndingTextPhase::CREDITS
                        : Mario::UIManager::EndingTextPhase::NONE;
    app.GetUIManager().SetEndingPhase(endPhase);
    app.GetUIManager().Update(Mario::UIManager::State::AXE_SEQUENCE);
}

}  // namespace Mario
