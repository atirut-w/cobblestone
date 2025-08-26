cmake_minimum_required(VERSION 3.30)
project(Cobblestone)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(cobblestone
    src/main.cpp
)
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
