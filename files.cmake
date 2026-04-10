set(SRC_FILES
    # Main application
    App.cpp

    # Core
    Mario/Camera.cpp
    Mario/PhysicsEngine.cpp
    Mario/SpritePathResolver.cpp

    # Level & Block
    Mario/Block.cpp
    Mario/Level.cpp

    # Player (MVC)
    Mario/PlayerState.cpp
    Mario/Player.cpp
    Mario/InputHandler.cpp

    # Entity (MVC)
    Mario/EntityState.cpp
    Mario/Entity.cpp
    Mario/EntityFactory.cpp

    # Behaviors (Phase 4 - Strategy Pattern)
    Mario/Behaviors/DefaultEntityBehavior.cpp
    Mario/Behaviors/EnemyBehavior.cpp
    Mario/Behaviors/AxeKoopaBehavior.cpp
    Mario/Behaviors/ParaKoopaBehavior.cpp
    Mario/Behaviors/PrincessBehavior.cpp
    Mario/Behaviors/ItemBehavior.cpp
    Mario/Behaviors/FireballBehavior.cpp
    Mario/Behaviors/BowserBehavior.cpp

    # Collision
    Mario/CollisionManager.cpp

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.cpp
    Mario/GameStateManager.cpp

    # Scene Handlers (Phase 5)
    Mario/TitleSceneHandler.cpp
    Mario/LoadingSceneHandler.cpp
    Mario/DeathSceneHandler.cpp
    Mario/GameOverSceneHandler.cpp
    Mario/ESCMenuSceneHandler.cpp

    # Managers (Phase 5)
    Mario/SceneManager.cpp
    Mario/UIManager.cpp
    Mario/AudioManager.cpp
    Mario/GameTheater.cpp

    # UI (Phase 5)
    Mario/FloatingText.cpp

    # Render
)

set(INCLUDE_FILES
    # Main application
    App.hpp

    # Core
    Mario/GameConfig.hpp
    Mario/Collider.hpp
    Mario/CollisionContext.hpp
    Mario/Camera.hpp
    Mario/PhysicsEngine.hpp
    Mario/SpritePathResolver.hpp

    # Level & Block
    Mario/EntityDef.hpp
    Mario/Block.hpp
    Mario/Level.hpp

    # Player (MVC)
    Mario/PlayerState.hpp
    Mario/Player.hpp
    Mario/InputHandler.hpp

    # Entity (MVC)
    Mario/EntityState.hpp
    Mario/Entity.hpp
    Mario/EntityFactory.hpp

    # Behaviors (Phase 4 - Strategy Pattern)
    Mario/Behaviors/IEntityBehavior.hpp
    Mario/Behaviors/DefaultEntityBehavior.hpp
    Mario/Behaviors/EnemyBehavior.hpp
    Mario/Behaviors/AxeKoopaBehavior.hpp
    Mario/Behaviors/ParaKoopaBehavior.hpp
    Mario/Behaviors/PrincessBehavior.hpp
    Mario/Behaviors/ItemBehavior.hpp
    Mario/Behaviors/FireballBehavior.hpp
    Mario/Behaviors/BowserBehavior.hpp

    # Collision
    Mario/CollisionManager.hpp

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.hpp
    Mario/GameStateManager.hpp

    # Scene Handlers (Phase 5)
    Mario/ISceneHandler.hpp
    Mario/TitleSceneHandler.hpp
    Mario/LoadingSceneHandler.hpp
    Mario/DeathSceneHandler.hpp
    Mario/GameOverSceneHandler.hpp
    Mario/ESCMenuSceneHandler.hpp

    # Managers (Phase 5)
    Mario/SceneManager.hpp
    Mario/UIManager.hpp
    Mario/AudioManager.hpp
    Mario/GameTheater.hpp

    # Audio Service (Phase 5)
    Mario/IAudioService.hpp

    # Infrastructure (Phase 5)
    Mario/EventSystem.hpp
    Mario/ServiceLocator.hpp

    # UI (Phase 5)
    Mario/FloatingText.hpp
    Mario/UIText.hpp
    Mario/UIImage.hpp
)

set(TEST_FILES
)
