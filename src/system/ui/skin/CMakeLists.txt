target_sources(
    NuVelocity PRIVATE
    AlphaSkinData.h
    AlphaSkinBorder.h
    ClassicSkinBorder.h
    JWindowSkin.h
    JWindowClassicTheme.h
    JWindowClassicTheme.cpp
    JWindowAlphaTheme.h
    JWindowAlphaTheme.cpp
    SkinAssets.cpp
)

target_include_directories(
    NuVelocity PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
