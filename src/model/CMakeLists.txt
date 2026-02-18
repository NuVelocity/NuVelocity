target_sources(
    NuVelocity PRIVATE
    Property.h
    ClassInfo.h
    Object.h
    ObjectRegistry.h
    PropertySerializer.h
    PropertySerializer.cpp
    Model.h
)

target_include_directories(
    NuVelocity PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
