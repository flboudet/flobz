cmake_minimum_required(VERSION 2.8)

IF (ENABLE_SDL12_BACKENDS)
ADD_LIBRARY(sdl_drawcontext STATIC
            common/SDL_IosFont.cpp
            common/SDL_AudioManager.cpp
            common/SDL_InputSwitch.cpp
            common/IosImgProcess.cpp
            sdl12/sdl12_drawcontext.cpp
            sdl12/sdl12_eventmanager.cpp
)

TARGET_INCLUDE_DIRECTORIES(sdl_drawcontext PRIVATE
                           .
                           common
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${SDL_INCLUDE_DIR}
)

TARGET_COMPILE_DEFINITIONS(sdl_drawcontext PRIVATE
                           HAVE_SDL_SDL_MIXER_H
                           HAVE_SDL_SDL_TTF_H
                           HAVE_SDL_SDL_IMAGE_H
)

ENDIF (ENABLE_SDL12_BACKENDS)

IF (ENABLE_SDL2_BACKENDS)
ADD_LIBRARY(sdl_drawcontext STATIC
            common/SDL_IosFont.cpp
            common/SDL_AudioManager.cpp
            common/SDL_InputSwitch.cpp
            common/IosImgProcess.cpp
            sdl13/sdl13_drawcontext.cpp
            sdl13/sdl13_eventmanager.cpp
)

TARGET_INCLUDE_DIRECTORIES(sdl_drawcontext PRIVATE
                           .
                           common
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${SDL2_INCLUDE_DIRS}
)

TARGET_COMPILE_DEFINITIONS(sdl_drawcontext PRIVATE
                           HAVE_SDL_SDL_MIXER_H
                           HAVE_SDL_SDL_TTF_H
                           HAVE_SDL_SDL_IMAGE_H
)

ENDIF (ENABLE_SDL2_BACKENDS)
