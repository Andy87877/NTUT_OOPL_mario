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

    # Collision
    Mario/CollisionManager.cpp

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.cpp
    Mario/GameStateManager.cpp
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

    # Collision
    Mario/CollisionManager.hpp

    # Level Completion (Phase 5)
    Mario/LevelCompleteController.hpp
    Mario/GameStateManager.hpp
)

set(TEST_FILES
)
