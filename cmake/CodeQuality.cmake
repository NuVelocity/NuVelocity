#------------------------------------------------------------------------------
# Clang Tidy
#------------------------------------------------------------------------------
option(NVE_CLANG_TIDY "Enable clang-tidy checks" ON)

if(MSVC)
    set(NVE_CLANG_TIDY OFF CACHE BOOL "" FORCE)
endif()

if(NVE_CLANG_TIDY)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy)

    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} CACHE STRING "clang-tidy executable")

        # Find run-clang-tidy script
        find_program(RUN_CLANG_TIDY_EXE
            NAMES run-clang-tidy run-clang-tidy.py
        )

        if(RUN_CLANG_TIDY_EXE)
            # Target to generate report
            add_custom_target(nve-clang-tidy-report
                COMMAND ${RUN_CLANG_TIDY_EXE}
                -p ${CMAKE_BINARY_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/src
                > ${CMAKE_CURRENT_SOURCE_DIR}/clang-tidy-report.txt
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Running clang-tidy and generating report..."
            )

            # Target to auto-fix
            add_custom_target(nve-clang-tidy-fix
                COMMAND ${RUN_CLANG_TIDY_EXE}
                -p ${CMAKE_BINARY_DIR}
                -fix ${CMAKE_CURRENT_SOURCE_DIR}/src
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Running clang-tidy with auto-fix..."
            )
        endif()
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

#------------------------------------------------------------------------------
# Clang Format
#------------------------------------------------------------------------------
option(NVE_CLANG_FORMAT "Enable clang-format targets" ON)

if(NVE_CLANG_FORMAT)
    find_program(CLANG_FORMAT_EXE NAMES clang-format)

    if(CLANG_FORMAT_EXE)
        message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")

        file(GLOB_RECURSE ALL_SOURCE_FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
        )

        # Target to check formatting
        add_custom_target(nve-clang-format-check
            COMMAND ${CLANG_FORMAT_EXE}
            --dry-run
            --Werror
            ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking code formatting with clang-format..."
        )

        # Target to fix formatting
        add_custom_target(nve-clang-format-fix
            COMMAND ${CLANG_FORMAT_EXE}
            -i
            ${ALL_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Fixing code formatting with clang-format..."
        )
    else()
        message(WARNING "clang-format requested but not found")
    endif()
endif()
