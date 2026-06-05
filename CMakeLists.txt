cmake_minimum_required(VERSION 3.16)
project(mod-talentmaster)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PRIVATE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

if (PRIVATE_SOURCES)
    add_library(${PROJECT_NAME} STATIC
        ${PRIVATE_SOURCES}
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            game-interface
    )

    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            FOLDER "mods"
    )

    add_dependencies(${PROJECT_NAME} revision.h)
endif()
