cmake_minimum_required(VERSION 3.0)

if (ENABLE_SDL12_BACKENDS)
add_library(sdl_drawcontext STATIC
            common/SDL_IosFont.cpp
            common/SDL_AudioManager.cpp
            common/SDL_InputSwitch.cpp
            common/IosImgProcess.cpp
            sdl12/sdl12_drawcontext.cpp
            sdl12/sdl12_eventmanager.cpp
)

target_include_directories(sdl_drawcontext PRIVATE
                           .
                           common
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${SDL_INCLUDE_DIR}
)

target_compile_definitions(sdl_drawcontext PRIVATE
                           HAVE_SDL_SDL_MIXER_H
                           HAVE_SDL_SDL_TTF_H
                           HAVE_SDL_SDL_IMAGE_H
)

endif (ENABLE_SDL12_BACKENDS)

if (ENABLE_SDL2_BACKENDS)
add_library(sdl_drawcontext STATIC
            common/SDL_IosFont.cpp
            common/SDL_AudioManager.cpp
            common/SDL_InputSwitch.cpp
            common/IosImgProcess.cpp
            sdl13/sdl13_drawcontext.cpp
            sdl13/sdl13_eventmanager.cpp
)

target_include_directories(sdl_drawcontext PRIVATE
                           .
                           common
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${SDL2_INCLUDE_DIRS}
)

target_compile_definitions(sdl_drawcontext PRIVATE
                           HAVE_SDL_SDL_MIXER_H
                           HAVE_SDL_SDL_TTF_H
                           HAVE_SDL_SDL_IMAGE_H
)

endif (ENABLE_SDL2_BACKENDS)

