target_sources(
    NuVelocity PRIVATE
    AssetManager.cpp
    AssetManager.h
    DecodeUtils.h
    AudioManager.cpp
    AudioManager.h
    Game.cpp
    Game.h
    GPUSpriteBatch.cpp
    GPUSpriteBatch.h
    LogCategory.h
    Manager.h
    Scene.h
    RendererSpriteBatch.cpp
    RendererSpriteBatch.h
    SpriteBatch.h
    Utils.h
)

target_include_directories(
    NuVelocity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(decoders)
