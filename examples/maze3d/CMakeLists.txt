cmake_minimum_required(VERSION 3.11)

project(maze3d)

add_executable(${PROJECT_NAME} main.c)

# Find SDL2 for audio only
include(FindPkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
)

# Link libraries - pspgl for OpenGL, SDL2 for audio
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    glut
    GLU
    GL
    pspvfpu
    m
)

# PSP-specific configuration
if(PSP)
    target_compile_options(${PROJECT_NAME} PRIVATE -O2)

    # Create EBOOT.PBP for PSP
    create_pbp_file(
        TARGET ${PROJECT_NAME}
        ICON_PATH NULL
        BACKGROUND_PATH NULL
        PREVIEW_PATH NULL
        TITLE "3D Maze"
        VERSION 01.00
    )
endif()
