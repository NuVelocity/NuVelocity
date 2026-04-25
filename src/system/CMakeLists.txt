if(NVE_GPU_SUPPORT)
    # Subdirectories for components
    add_subdirectory(shaders)

    # Ensure shaders are generated before compiling the main library
    add_dependencies(NuVelocity NuVelocityShaders)

    target_sources(
        NuVelocity PRIVATE
        GPUSpriteBatch.cpp
        GPUSpriteBatch.h
    )
endif()

target_sources(
    NuVelocity PRIVATE
    AssetExporter.cpp
    AssetExporter.h
    AssetManager.cpp
    AssetManager.h
    AudioManager.cpp
    AudioManager.h
    InputManager.cpp
    InputManager.h
    FontManager.cpp
    FontManager.h
    Game.cpp
    Game.h
    LogCategory.h
    Manager.h
    Scene.h
    RendererSpriteBatch.cpp
    RendererSpriteBatch.h
    SpriteBatch.h
    SpriteBatch.cpp
    Utils.h
)

target_include_directories(
    NuVelocity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(decoders)
add_subdirectory(ui)
