set(SRC_FILES
    # Main application
    App.cpp

    # Core
    Mario/Core/Camera.cpp
    Mario/Core/PhysicsEngine.cpp
    Mario/Core/SpritePathResolver.cpp

    # Player (MVC)
    Mario/Player/PlayerState.cpp
    Mario/Player/PlayerForm.cpp
    Mario/Player/PlayerDeathAnimation.cpp
    Mario/Player/Player.cpp

    # Level & Block
    Mario/Level/Level.cpp
    Mario/Level/Block.cpp
    Mario/Level/MovingPlatform.cpp
    Mario/Level/EntityState.cpp
    Mario/Level/EnemyDeathAnimation.cpp
    Mario/Level/EnemyDeathStyleFactory.cpp
    Mario/Level/Entity.cpp
    Mario/Level/EntityFactory.cpp
    Mario/Level/LevelCompleteController.cpp
    Mario/Level/GameStateManager.cpp

    # Behaviors (Phase 4 - Strategy Pattern)
    Mario/Behaviors/DefaultEntityBehavior.cpp
    Mario/Behaviors/EnemyBehavior.cpp
    Mario/Behaviors/KoopaFamily.cpp
    Mario/Behaviors/StaticEntityBehaviors.cpp
    Mario/Behaviors/ItemBehavior.cpp
    Mario/Behaviors/FireballBehavior.cpp
    Mario/Behaviors/BowserBehavior.cpp
    Mario/Behaviors/CastleFireSpawnerBehavior.cpp
    Mario/Behaviors/PiranhaPlantBehavior.cpp
    Mario/Behaviors/PodobooBehavior.cpp
    Mario/Behaviors/ParticleDebris.cpp

    # Collision (OOP subsystem)
    Mario/CollisionManager.cpp
    Mario/Collision/BlockContactResolver.cpp
    Mario/Collision/PlayerBlockHandler.cpp
    Mario/Collision/PlayerEntityHandler.cpp
    Mario/Collision/EntityBlockHandler.cpp
    Mario/Collision/EntityEntityHandler.cpp

    # Scene Handlers (State Pattern)
    Mario/Scenes/MenuSceneHandlers.cpp
    Mario/Scenes/LoadingSceneHandler.cpp
    Mario/Scenes/PlayingSceneHandler.cpp
    Mario/Scenes/FlagpoleSceneHandler.cpp
    Mario/Scenes/PipeWarpSceneHandler.cpp
    Mario/Scenes/AxeSequenceSceneHandler.cpp
    Mario/Scenes/ESCMenuSceneHandler.cpp

    # UI & Panels
    Mario/UI/UIManager.cpp
    Mario/UI/CoinUI.cpp
    Mario/UI/FloatingText.cpp

    # Services (DIP & Services)
    Mario/Services/InputHandler.cpp
    Mario/Services/AudioPathResolver.cpp
    Mario/Services/AudioManager.cpp
)

set(INCLUDE_FILES
    # Main application
    App.hpp

    # Core
    Mario/Core/Collider.hpp
    Mario/Core/CollisionContext.hpp
    Mario/Core/Camera.hpp
    Mario/Core/PhysicsEngine.hpp
    Mario/Core/SpritePathResolver.hpp
    Mario/Core/GameConfig.hpp

    # Player (MVC)
    Mario/Player/Player.hpp
    Mario/Player/PlayerState.hpp
    Mario/Player/PlayerForm.hpp
    Mario/Player/PlayerDeathAnimation.hpp

    # Level & Block
    Mario/Level/Level.hpp
    Mario/Level/Block.hpp
    Mario/Level/MovingPlatform.hpp
    Mario/Level/EntityDef.hpp
    Mario/Level/Entity.hpp
    Mario/Level/EntityFactory.hpp
    Mario/Level/EntityState.hpp
    Mario/Level/EnemyDeathAnimation.hpp
    Mario/Level/EnemyDeathStyleFactory.hpp
    Mario/Level/LevelCompleteController.hpp
    Mario/Level/GameStateManager.hpp

    # Behaviors (Phase 4 - Strategy Pattern)
    Mario/Behaviors/IEntityBehavior.hpp
    Mario/Behaviors/DefaultEntityBehavior.hpp
    Mario/Behaviors/EnemyBehavior.hpp
    Mario/Behaviors/KoopaFamily.hpp
    Mario/Behaviors/StaticEntityBehaviors.hpp
    Mario/Behaviors/ItemBehavior.hpp
    Mario/Behaviors/FireballBehavior.hpp
    Mario/Behaviors/BowserBehavior.hpp
    Mario/Behaviors/CastleFireSpawnerBehavior.hpp
    Mario/Behaviors/PiranhaPlantBehavior.hpp
    Mario/Behaviors/PodobooBehavior.hpp
    Mario/Behaviors/ParticleDebris.hpp

    # Collision
    Mario/CollisionManager.hpp
    Mario/Collision/ICollisionHandler.hpp
    Mario/Collision/BlockContactResolver.hpp
    Mario/Collision/PlayerBlockHandler.hpp
    Mario/Collision/PlayerEntityHandler.hpp
    Mario/Collision/EntityBlockHandler.hpp
    Mario/Collision/EntityEntityHandler.hpp

    # Scene Handlers (State Pattern)
    Mario/Scenes/ISceneHandler.hpp
    Mario/Scenes/MenuSceneHandlers.hpp
    Mario/Scenes/LoadingSceneHandler.hpp
    Mario/Scenes/PlayingSceneHandler.hpp
    Mario/Scenes/FlagpoleSceneHandler.hpp
    Mario/Scenes/PipeWarpSceneHandler.hpp
    Mario/Scenes/AxeSequenceSceneHandler.hpp
    Mario/Scenes/ESCMenuSceneHandler.hpp

    # UI & Panels
    Mario/UI/UIPanel.hpp
    Mario/UI/UIManager.hpp
    Mario/UI/UIWidgets.hpp
    Mario/UI/CoinUI.hpp
    Mario/UI/FloatingText.hpp

    # Services (DIP & Services)
    Mario/Services/ServiceLocator.hpp
    Mario/Services/EventSystem.hpp
    Mario/Services/IInputHandler.hpp
    Mario/Services/InputHandler.hpp
    Mario/Services/IAudioService.hpp
    Mario/Services/AudioType.hpp
    Mario/Services/AudioPathResolver.hpp
    Mario/Services/AudioManager.hpp
)

set(TEST_FILES
)
