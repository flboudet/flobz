PROJECT(flobz C CXX)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(SDL REQUIRED)


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
                      ${SDL_LIBRARY}
                     )
