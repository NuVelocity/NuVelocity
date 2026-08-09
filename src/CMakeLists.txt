add_subdirectory(model)
add_subdirectory(system)

set(NVE_OBJECT_TYPES_DEF ${CMAKE_CURRENT_SOURCE_DIR}/EngineObjectTypes.def)
set(NVE_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(NVE_OBJECT_REGISTRATION_CPP ${NVE_GENERATED_DIR}/ObjectRegistration.generated.cpp)

file(MAKE_DIRECTORY ${NVE_GENERATED_DIR})

add_custom_command(
    OUTPUT ${NVE_OBJECT_REGISTRATION_CPP}
    COMMAND ${CMAKE_COMMAND} 
        -DINPUT_DEF_FILE="${NVE_OBJECT_TYPES_DEF}"
        -DOUTPUT_CPP_FILE="${NVE_OBJECT_REGISTRATION_CPP}"
        -DHEADER_NAME="ObjectRegistration.h"
        -DNAMESPACE_NAME="nuvelocity"
        -DFUNCTION_NAME="RegisterEngineObjectTypes"
        -P "${PROJECT_SOURCE_DIR}/cmake/GenerateObjectRegistration.cmake"
    DEPENDS ${NVE_OBJECT_TYPES_DEF} ${PROJECT_SOURCE_DIR}/cmake/GenerateObjectRegistration.cmake
    COMMENT "Generating ObjectRegistration.generated.cpp from .def file"
)

add_custom_target(NuVelocityObjGen DEPENDS ${NVE_OBJECT_REGISTRATION_CPP})
add_dependencies(NuVelocity NuVelocityObjGen)
set_source_files_properties(${NVE_OBJECT_REGISTRATION_CPP} PROPERTIES GENERATED TRUE)

target_sources(
    NuVelocity PRIVATE
    BlitType.h
    BlitTypeConverter.h
    Colors.h
    Collidable2D.h
    EngineObjectTypes.def
    ObjectRegistration.h
    ${NVE_OBJECT_REGISTRATION_CPP}
    Font.cpp
    Font.h
    ParticleType.cpp
    ParticleType.h
    ParticleGeneratorInfo.cpp
    ParticleGeneratorInfo.h
    FontBitmap.cpp
    FontBitmap.h
    FrameInfo.cpp
    FrameInfo.h
    Frame.cpp
    Frame.h
    Image.cpp
    Image.h
    SequenceFlags.h
    Sequence.cpp
    Sequence.h
    SequenceFrameInfoList.cpp
    SequenceFrameInfoList.h
    StandAloneFrame.cpp
    StandAloneFrame.h
    Particle.cpp
    Particle.h
    ParticleGenerator.cpp
    ParticleGenerator.h
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
