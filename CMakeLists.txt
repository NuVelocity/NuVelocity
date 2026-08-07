# NuVelocity Engine; a re-implementation of the Velocity Engine under SDL.
#
# Please see the file LICENSE in the source's root directory.

set(NVE_VERSION 0.1.0)
set(NVE_SOVERSION 1)

cmake_minimum_required(VERSION 3.13.0)

project(NuVelocity VERSION ${NVE_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(NVE_RESTORE_TGA "Enable re-exporting TGA files from cache" ON)

# ------------------------------------------------------------------------------
# Third-Party Libraries
# ------------------------------------------------------------------------------
option(NVE_VENDORED "Use vendored libraries" ON)

if(NVE_VENDORED)
    # * SDL3
    add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

    # Use a reduced SDL image feature set.
    set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_BMP OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_GIF OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_LBM OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_PCX OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_PNM OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_QOI OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_XCF OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_XPM OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_XV OFF CACHE BOOL "" FORCE)

    # * SDL_Image
    add_subdirectory(third_party/SDL_image EXCLUDE_FROM_ALL)

    # * SDL_Mixer
    add_subdirectory(third_party/SDL_mixer EXCLUDE_FROM_ALL)

    # Disable FLAC support on MSVC:
    # upstream support has a broken CMake config that I cba to fix yet.
    if(MSVC)
        set(SDLMIXER_FLAC_LIBFLAC OFF CACHE BOOL "" FORCE)
    endif()

    # * SDL_TTF
    add_subdirectory(third_party/SDL_ttf EXCLUDE_FROM_ALL)

    # * zlib
    # Avoid conflict with vendored SDL zlib on MSVC.
    if(NOT MSVC)
        add_subdirectory(third_party/zlib EXCLUDE_FROM_ALL)
    endif()
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
    find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image-shared)
    find_package(SDL3_mixer REQUIRED CONFIG REQUIRED COMPONENTS SDL3_mixer-shared)
    find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-shared)
    find_package(ZLIB REQUIRED)
endif()

find_program(GLSLANG_VALIDATOR glslangValidator)

if(GLSLANG_VALIDATOR)
    set(NVE_GPU_SUPPORT ON)
endif()

# PhysicsFS is vendored because we patch it to add support for our
# custom archive format.
set(PHYSFS_BUILD_TEST OFF CACHE BOOL "" FORCE)

# Turn off unneeded archive formats.
set(PHYSFS_ARCHIVE_GRP OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_WAD OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_CSM OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_HOG OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_MVL OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_QPAK OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_ROFS OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_SLB OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_ISO9660 OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_VDF OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_LECARCHIVES OFF CACHE BOOL "" FORCE)
set(PHYSFS_ARCHIVE_POD OFF CACHE BOOL "" FORCE)

if(NOT MSVC)
    # Enable SDL3 platform support.
    set(PHYSFS_PLATFORM_SDL3 ON CACHE BOOL "" FORCE)
endif()

# Disable building as a static library.
set(PHYSFS_BUILD_STATIC OFF CACHE BOOL "" FORCE)
# Disable installing the library, since we link it into our own library.
set(PHYSFS_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/physfs EXCLUDE_FROM_ALL)

# ------------------------------------------------------------------------------
# Third-Party Libraries (via FetchContent)
# ------------------------------------------------------------------------------
include(FetchContent)

# * argparse
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v3.2
)
FetchContent_MakeAvailable(argparse)

# ------------------------------------------------------------------------------
# Code Quality Tools
# ------------------------------------------------------------------------------
include(cmake/CodeQuality.cmake)

# ------------------------------------------------------------------------------
# NuVelocity Library
# ------------------------------------------------------------------------------
option(NVE_BUILD_STATIC "Enable building static library" OFF)

if(NVE_BUILD_STATIC)
    add_library(NuVelocity STATIC)
else()
    add_library(NuVelocity SHARED)
endif()

add_library(NuVelocity::NuVelocity ALIAS NuVelocity)

target_compile_definitions(
    NuVelocity PRIVATE
    DEBUG
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