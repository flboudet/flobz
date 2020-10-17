PROJECT(flobz C CXX)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(SDL)
FIND_PACKAGE(SDL_mixer)
FIND_PACKAGE(SDL_image)
FIND_PACKAGE(SDL_ttf)
FIND_PACKAGE(SDL2)
#set(SDL2_PREFIX "/usr")
#set(SDL2_EXEC_PREFIX "/usr")
#set(SDL2_INCLUDE_DIRS "${prefix}/include/SDL2")
#set(SDL2_LIBRARIES " -lSDL2")

OPTION(ENABLE_SDL2_BACKENDS "Enable SDL 2 backends" OFF)
OPTION(ENABLE_SDL12_BACKENDS "Enable SDL 1.2 backends" OFF)
OPTION(ENABLE_NULL_BACKENDS  "Enable NULL backends" OFF)

ADD_SUBDIRECTORY(iosfc)
ADD_SUBDIRECTORY(goomsl)
ADD_SUBDIRECTORY(gametools)
ADD_SUBDIRECTORY(fpcore)

ADD_EXECUTABLE(flobz
               fpcore/main.cpp
              )

TARGET_INCLUDE_DIRECTORIES(flobz PRIVATE
                           fpcore
                           gametools
                           iosfc
                          )

TARGET_LINK_LIBRARIES(flobz PRIVATE
                      fpcore
                      gametools
                      goomsl
                      iosfc
                     )

IF (ENABLE_SDL12_BACKENDS)
TARGET_LINK_LIBRARIES(flobz PRIVATE
                      sdl_drawcontext
                      ${SDL_IMAGE_LIBRARIES}
                      ${SDL_TTF_LIBRARIES}
                      ${SDL_MIXER_LIBRARIES}
                      ${SDL_LIBRARY}
                     )
ENDIF (ENABLE_SDL12_BACKENDS)
