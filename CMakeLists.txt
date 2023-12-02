project(flobz C CXX)

cmake_minimum_required(VERSION 3.0)

include(GNUInstallDirs)
include(FindPkgConfig)

option(ENABLE_SDL2_BACKENDS "Enable SDL 2 backends" OFF)
option(ENABLE_SDL12_BACKENDS "Enable SDL 1.2 backends" OFF)
option(ENABLE_NULL_BACKENDS  "Enable NULL backends" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   set(SDL2_IMAGE_LIBRARIES)
   set(SDL2_TTF_LIBRARIES)
   set(SDL2_MIXER_LIBRARIES)
   set(SDL2_LIBRARIES)

   set(CMAKE_CXX_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2")
else ()


if (ENABLE_SDL12_BACKENDS)
   find_package(SDL)
   find_package(SDL_mixer)
   find_package(SDL_image)
   find_package(SDL_ttf)
endif()

if (ENABLE_SDL2_BACKENDS)
   find_package(SDL2)
   pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer)
   pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image)
   pkg_search_module(SDL2_TTF REQUIRED SDL2_ttf)
endif()

endif ()

add_subdirectory(iosfc)
add_subdirectory(goomsl)
add_subdirectory(gametools)
add_subdirectory(fpcore)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   add_executable(flobz
                  fpcore/main_emscripten.cpp
                 )
else ()
   add_executable(flobz
                    fpcore/main.cpp
                   )
endif ()


target_include_directories(flobz PRIVATE
                           fpcore
                           gametools
                           iosfc
                          )

target_link_libraries(flobz PRIVATE
                      fpcore
                      gametools
                      goomsl
                      iosfc
                     )

if (ENABLE_SDL12_BACKENDS)
target_link_libraries(flobz PRIVATE
                      sdl_drawcontext
                      ${SDL_IMAGE_LIBRARIES}
                      ${SDL_TTF_LIBRARIES}
                      ${SDL_MIXER_LIBRARIES}
                      ${SDL_LIBRARY}
                     )
ENDIF (ENABLE_SDL12_BACKENDS)

if (ENABLE_SDL2_BACKENDS)
target_link_libraries(flobz PRIVATE
                      sdl_drawcontext
                      ${SDL2_IMAGE_LIBRARIES}
                      ${SDL2_TTF_LIBRARIES}
                      ${SDL2_MIXER_LIBRARIES}
                      ${SDL2_LIBRARIES}
                     )
endif (ENABLE_SDL2_BACKENDS)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
   set_target_properties(flobz PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/data@/data --use-preload-plugins -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_MIXER=2 -s USE_SDL_TTF=2")
   set_target_properties(flobz PROPERTIES SUFFIX ".html")
endif ()
