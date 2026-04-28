add_subdirectory(model)
add_subdirectory(system)

set(NVE_OBJECT_TYPES_DEF ${CMAKE_CURRENT_SOURCE_DIR}/EngineObjectTypes.def)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${NVE_OBJECT_TYPES_DEF})

file(STRINGS ${NVE_OBJECT_TYPES_DEF} NVE_OBJECT_TYPE_LINES
    REGEX "^NVE_REGISTER_TYPE\\([A-Za-z_][A-Za-z0-9_]*\\)$")

set(NVE_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(NVE_OBJECT_REGISTRATION_CPP ${NVE_GENERATED_DIR}/ObjectRegistration.generated.cpp)
file(MAKE_DIRECTORY ${NVE_GENERATED_DIR})

file(WRITE ${NVE_OBJECT_REGISTRATION_CPP} "#include \"ObjectRegistration.h\"\n\n")

foreach(_line IN LISTS NVE_OBJECT_TYPE_LINES)
    string(REGEX REPLACE "^NVE_REGISTER_TYPE\\(([A-Za-z_][A-Za-z0-9_]*)\\)$" "\\1" _type ${_line})
    file(APPEND ${NVE_OBJECT_REGISTRATION_CPP} "#include \"${_type}.h\"\n")
endforeach()

file(APPEND ${NVE_OBJECT_REGISTRATION_CPP}
    "\n#include \"model/ObjectRegistry.h\"\n\nnamespace nuvelocity\n{\n    void RegisterEngineObjectTypes()\n    {\n")

foreach(_line IN LISTS NVE_OBJECT_TYPE_LINES)
    string(REGEX REPLACE "^NVE_REGISTER_TYPE\\(([A-Za-z_][A-Za-z0-9_]*)\\)$" "\\1" _type ${_line})
    file(APPEND ${NVE_OBJECT_REGISTRATION_CPP}
        "        ObjectRegistry::Get().Register(${_type}::GetClassInfo());\n")
endforeach()

file(APPEND ${NVE_OBJECT_REGISTRATION_CPP} "    }\n} // namespace nuvelocity\n")
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
