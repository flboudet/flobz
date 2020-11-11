PROJECT(flobz C CXX)

cmake_minimum_required(VERSION 2.8)

INCLUDE(GNUInstallDirs)
INCLUDE(FindPkgConfig)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   set(SDL2_IMAGE_LIBRARIES)
   set(SDL2_TTF_LIBRARIES)
   set(SDL2_MIXER_LIBRARIES)
   set(SDL2_LIBRARIES)

   set(CMAKE_CXX_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2")
else ()


FIND_PACKAGE(SDL)
FIND_PACKAGE(SDL_mixer)
FIND_PACKAGE(SDL_image)
FIND_PACKAGE(SDL_ttf)

FIND_PACKAGE(SDL2)
PKG_SEARCH_MODULE(SDL2_MIXER REQUIRED SDL2_mixer)
PKG_SEARCH_MODULE(SDL2_IMAGE REQUIRED SDL2_image)
PKG_SEARCH_MODULE(SDL2_TTF REQUIRED SDL2_ttf)

endif ()

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

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   ADD_EXECUTABLE(flobz
                  fpcore/main_emscripten.cpp
                 )
else ()
     ADD_EXECUTABLE(flobz
                    fpcore/main.cpp
                   )
endif ()


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

IF (ENABLE_SDL2_BACKENDS)
TARGET_LINK_LIBRARIES(flobz PRIVATE
                      sdl_drawcontext
                      ${SDL2_IMAGE_LIBRARIES}
                      ${SDL2_TTF_LIBRARIES}
                      ${SDL2_MIXER_LIBRARIES}
                      ${SDL2_LIBRARIES}
                     )
ENDIF (ENABLE_SDL2_BACKENDS)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   set_target_properties(flobz PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/data@/data --use-preload-plugins -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2")
   set_target_properties(flobz PROPERTIES SUFFIX ".html")
endif ()
