# NuVelocity Engine; a re-implementation of the Velocity Engine under SDL.
#
# Please see the file LICENSE in the source's root directory.

set(NVE_VERSION 0.1.0)
set(NVE_SOVERSION 1)

cmake_minimum_required(VERSION 3.25.0)

project(NuVelocity VERSION ${NVE_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NVE_RESTORE_TGA "Enable re-exporting TGA files from cache" ON)
option(NVE_BUILD_SHARED "Enable building shared library" OFF)
option(NVE_BUILD_STATIC "Enable building static library" ON)

if(NVE_BUILD_SHARED AND NVE_BUILD_STATIC)
    message(FATAL_ERROR "NVE_BUILD_SHARED and NVE_BUILD_STATIC cannot both be enabled - pick one")
endif()
if(NOT (NVE_BUILD_SHARED OR NVE_BUILD_STATIC))
    message(FATAL_ERROR "NVE_BUILD_SHARED and NVE_BUILD_STATIC cannot both be disabled")
endif()

set(BUILD_SHARED_LIBS ${NVE_BUILD_SHARED} CACHE BOOL "" FORCE)

# ------------------------------------------------------------------------------
# Third-Party Libraries
# ------------------------------------------------------------------------------
option(NVE_VENDORED "Use vendored libraries" ON)

if(NVE_VENDORED)
    set(SDL_SHARED ${NVE_BUILD_SHARED} CACHE BOOL "" FORCE)
    set(SDL_STATIC ${NVE_BUILD_STATIC} CACHE BOOL "" FORCE)

    # * SDL3
    add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

    # * SDL_Image
    # Turn off unneeded image formats.
    foreach(_nve_fmt AVIF BMP GIF LBM PCX PNM QOI WEBP XCF XPM XV)
        set(SDLIMAGE_${_nve_fmt} OFF CACHE BOOL "" FORCE)
    endforeach()

    add_subdirectory(third_party/SDL_image EXCLUDE_FROM_ALL)

    # * SDL_Mixer
    # Disable FLAC support on MSVC:
    # upstream support has a broken CMake config that I cba to fix yet.
    if(MSVC)
        set(SDLMIXER_FLAC_LIBFLAC OFF CACHE BOOL "" FORCE)
    endif()

    add_subdirectory(third_party/SDL_mixer EXCLUDE_FROM_ALL)

    # * SDL_TTF
    add_subdirectory(third_party/SDL_ttf EXCLUDE_FROM_ALL)

    # * zlib
    # Avoid conflict with vendored SDL zlib on MSVC.
    if(NOT MSVC)
        set(ZLIB_BUILD_SHARED ${NVE_BUILD_SHARED} CACHE BOOL "" FORCE)
        set(ZLIB_BUILD_STATIC ${NVE_BUILD_STATIC} CACHE BOOL "" FORCE)
        set(ZLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(ZLIB_INSTALL OFF CACHE BOOL "" FORCE)
        add_subdirectory(third_party/zlib EXCLUDE_FROM_ALL)

        if(NVE_BUILD_STATIC AND NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        endif()
    endif()
else()
    if(NVE_BUILD_STATIC)
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-static)
        find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image-static)
        find_package(SDL3_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL3_mixer-static)
        find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-static)
    else()
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
        find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image-shared)
        find_package(SDL3_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL3_mixer-shared)
        find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-shared)
    endif()
    find_package(ZLIB REQUIRED)
endif()

find_program(GLSLANG_VALIDATOR glslangValidator)

if(GLSLANG_VALIDATOR)
    set(NVE_GPU_SUPPORT ON)
endif()

# * PhysicsFS
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_STATIC ${NVE_BUILD_STATIC} CACHE BOOL "" FORCE)
set(PHYSFS_BUILD_SHARED ${NVE_BUILD_SHARED} CACHE BOOL "" FORCE)
set(PHYSFS_INSTALL OFF CACHE BOOL "" FORCE)

# Turn off unneeded archive formats.
foreach(_nve_fmt GRP WAD CSM HOG MVL QPAK ROFS SLB ISO9660 VDF LECARCHIVES POD)
    set(PHYSFS_ARCHIVE_${_nve_fmt} OFF CACHE BOOL "" FORCE)
endforeach()

if(NOT MSVC)
    # Enable SDL3 platform support.
    set(PHYSFS_PLATFORM_SDL3 ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(third_party/physfs EXCLUDE_FROM_ALL)

add_subdirectory(third_party/argparse EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------------------
# Code Quality Tools
# ------------------------------------------------------------------------------
include(cmake/CodeQuality.cmake)

# ------------------------------------------------------------------------------
# NuVelocity Library
# ------------------------------------------------------------------------------
if(NVE_BUILD_STATIC)
    add_library(NuVelocity STATIC)
else()
    add_library(NuVelocity SHARED)
endif()

add_library(NuVelocity::NuVelocity ALIAS NuVelocity)

target_compile_definitions(
    NuVelocity PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    NVE_LIB
)

if(NVE_RESTORE_TGA)
    target_compile_definitions(NuVelocity PRIVATE NVE_RESTORE_TGA)
endif()

if(NVE_GPU_SUPPORT)
    target_compile_definitions(NuVelocity PRIVATE NVE_GPU_SUPPORT)
endif()

target_link_libraries(
    NuVelocity PUBLIC
    ZLIB::ZLIB
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    PhysFS::PhysFS
    argparse
)

target_sources(
    NuVelocity PRIVATE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/physfs/extras
    FILES third_party/physfs/extras/physfssdl3.h
)

target_sources(
    NuVelocity PRIVATE
    third_party/physfs/extras/physfssdl3.c
)

if(NOT MSVC OR NOT NVE_BUILD_STATIC)
    set_property(TARGET NuVelocity PROPERTY OUTPUT_NAME "nuvelocity")
endif()

set_property(TARGET NuVelocity PROPERTY MACOSX_RPATH 1)
set_property(TARGET NuVelocity PROPERTY VERSION ${NVE_VERSION})
set_property(TARGET NuVelocity PROPERTY SOVERSION ${NVE_SOVERSION})

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_subdirectory(src)