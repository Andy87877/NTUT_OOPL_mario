set(SRC_FILES
    # Main application
    App.cpp

    # Core
    Mario/Camera.cpp
    Mario/PhysicsEngine.cpp
    Mario/SpritePathResolver.cpp

    # Level & Block
    Mario/Block.cpp
    Mario/MovingPlatform.cpp
    Mario/Level.cpp

    # Player (MVC)
    Mario/PlayerState.cpp
    Mario/PlayerDeathAnimation.cpp
    Mario/Player.cpp
    Mario/InputHandler.cpp

    # Entity (MVC)
    Mario/EntityState.cpp
    Mario/EnemyDeathAnimation.cpp
    Mario/EnemyDeathStyleFactory.cpp
    Mario/Entity.cpp
    Mario/EntityFactory.cpp

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
    # CollisionManager is a thin facade; logic lives in Collision/ handlers.
    Mario/CollisionManager.cpp
    Mario/Collision/BlockContactResolver.cpp
    Mario/Collision/PlayerBlockHandler.cpp
    Mario/Collision/PlayerEntityHandler.cpp
    Mario/Collision/EntityBlockHandler.cpp
    Mario/Collision/EntityEntityHandler.cpp

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.cpp
    Mario/GameStateManager.cpp

    # Scene Handlers (State Pattern)
    Mario/MenuSceneHandlers.cpp
    Mario/LoadingSceneHandler.cpp
    Mario/PlayingSceneHandler.cpp
    Mario/FlagpoleSceneHandler.cpp
    Mario/PipeWarpSceneHandler.cpp
    Mario/AxeSequenceSceneHandler.cpp
    Mario/ESCMenuSceneHandler.cpp

    # Managers
    # UIManager.cpp also contains CoinUI + FloatingText implementations
    #   (they are private sub-components of UIManager; no external consumers)
    Mario/UIManager.cpp
    # AudioManager.cpp also contains AudioPathResolver implementation
    #   (AudioPathResolver is an internal helper used only by AudioManager)
    Mario/AudioManager.cpp
    # SceneManager / GameTheater removed — superseded by App + ISceneHandler State Pattern

    # Render
)

set(INCLUDE_FILES
    # Main application
    App.hpp

    # Core
    Mario/GameConfig.hpp
    Mario/Collider.hpp
    Mario/Camera.hpp
    Mario/PhysicsEngine.hpp
    Mario/SpritePathResolver.hpp

    # Level & Block
    Mario/EntityDef.hpp
    Mario/Block.hpp
    Mario/Level.hpp

    # Player (MVC)
    Mario/PlayerState.hpp
    Mario/PlayerDeathAnimation.hpp
    Mario/Player.hpp
    Mario/InputHandler.hpp

    # Entity (MVC)
    Mario/EntityState.hpp
    Mario/EnemyDeathAnimation.hpp
    Mario/EnemyDeathStyleFactory.hpp
    Mario/Entity.hpp
    Mario/EntityFactory.hpp

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

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.hpp
    Mario/GameStateManager.hpp

    # Scene Handlers (Phase 5)
    Mario/ISceneHandler.hpp
    Mario/MenuSceneHandlers.hpp
    Mario/LoadingSceneHandler.hpp
    Mario/PlayingSceneHandler.hpp
    Mario/FlagpoleSceneHandler.hpp
    Mario/PipeWarpSceneHandler.hpp
    Mario/AxeSequenceSceneHandler.hpp
    Mario/ESCMenuSceneHandler.hpp

    # Managers
    # AudioManager.hpp contains all audio sub-system declarations:
    #   BGMName/SFXName enums (was AudioType.hpp)
    #   IAudioService interface (was IAudioService.hpp)
    #   AudioPathResolver helper (was AudioPathResolver.hpp)
    #   AudioManager singleton
    Mario/AudioManager.hpp
    # SceneManager.hpp / GameTheater.hpp removed — superseded by App + ISceneHandler State Pattern
    Mario/UIManager.hpp

    # UI components
    # UIWidgets.hpp contains UIText + UIImage (merged — both exclusively used by HUD sub-system)
    Mario/UIWidgets.hpp
    Mario/FloatingText.hpp
    Mario/CoinUI.hpp
)

set(TEST_FILES
)
