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

    # Collision
    Mario/CollisionManager.cpp
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

    # Collision
    Mario/CollisionManager.hpp
)

set(TEST_FILES
)
