# List of shaders to process
set(SHADER_SOURCES
    sprite.vert.glsl
    sprite.frag.glsl
)

set(GENERATED_SHADER_HEADERS)

foreach(SHADER_SRC ${SHADER_SOURCES})
    # From sprite.vert.glsl, we want NAME=sprite, STAGE=vert
    if(SHADER_SRC MATCHES "^([^.]+)\\.([^.]+)\\.glsl$")
        string(REGEX MATCH "^([^.]+)\\.([^.]+)\\.glsl$" MATCH_RESULT ${SHADER_SRC})
        set(SHADER_BASE_NAME ${CMAKE_MATCH_1})
        set(SHADER_STAGE ${CMAKE_MATCH_2})
    else()
        message(FATAL_ERROR "Shader filename ${SHADER_SRC} does not match expected pattern <base>.<stage>.glsl")
    endif()

    set(SPV_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_BASE_NAME}.${SHADER_STAGE}.spv")
    set(HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_BASE_NAME}_${SHADER_STAGE}_spv.h")

    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SRC} -o ${SPV_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_SRC}
        COMMENT "Compiling shader ${SHADER_SRC} to SPIR-V"
        VERBATIM
    )

    add_custom_command(
        OUTPUT ${HEADER_FILE}
        COMMAND ${CMAKE_COMMAND} 
                -DINPUT_FILE=${SPV_FILE} 
                -DOUTPUT_FILE=${HEADER_FILE} 
                -DVAR_NAME=${SHADER_BASE_NAME}_${SHADER_STAGE}_spv
                -P "${NuVelocity_SOURCE_DIR}/cmake/GenerateShaderHeader.cmake"
        DEPENDS ${SPV_FILE} "${NuVelocity_SOURCE_DIR}/cmake/GenerateShaderHeader.cmake"
        COMMENT "Generating header ${HEADER_FILE} from SPIR-V"
        VERBATIM
    )

    set_source_files_properties(${HEADER_FILE} PROPERTIES GENERATED TRUE)
    list(APPEND GENERATED_SHADER_HEADERS ${HEADER_FILE})
endforeach()

# Create a custom target for shaders instead of an interface library
# This ensures CMake doesn't try to "find" the headers during configuration step
add_custom_target(NuVelocityShaders 
    DEPENDS ${GENERATED_SHADER_HEADERS}
    SOURCES ${SHADER_SOURCES}
)

# Export include path for #include "shaders/..."
set(NU_VELOCITY_SHADER_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/.." PARENT_SCOPE)
