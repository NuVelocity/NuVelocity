add_subdirectory(model)
add_subdirectory(system)

target_sources(
    NuVelocity PRIVATE
    BlitType.h
    StandAloneFrame.cpp
    StandAloneFrame.h
    PaletteHolder.cpp
    PaletteHolder.h
    ModuleInfo.cpp
    ModuleInfo.h
    API.h
)

target_include_directories(
    NuVelocity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
