target_sources(
    NuVelocity PRIVATE
    AssetManager.cpp
    AssetManager.h
    AudioManager.cpp
    AudioManager.h
    Game.cpp
    Game.h
    LogCategory.h
    Manager.h
    Scene.h
    Utils.h
)

target_include_directories(
    NuVelocity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
