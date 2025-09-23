cmake_minimum_required(VERSION 3.30)
project(Cobblestone)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(3rd)

include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)

add_executable(cobblestone
    src/main.cpp
    src/font.cpp
)
target_include_directories(cobblestone PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(cobblestone PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    nlohmann_json::nlohmann_json
    utf8cpp
    GL
)

if (PSP)
    target_link_libraries(cobblestone PRIVATE
        pspdebug
        pspdisplay
        pspge
    )

    create_pbp_file(
        TARGET cobblestone
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE "Cobblestone Engine"
        VERSION 01.00
    )
endif()
