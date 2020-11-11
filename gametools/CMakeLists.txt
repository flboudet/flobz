cmake_minimum_required(VERSION 2.8)

ADD_LIBRARY(gametools STATIC
            GTLog.cpp
            GameCursor.cpp
            gameloop.cpp
            gameui.cpp
            GameStateMachine.cpp
            ListView.cpp
            Frame.cpp
            FramedButton.cpp
            FramedEditField.cpp
	    mt19937ar.cpp
            SwitchedButton.cpp
            RadioButton.cpp
            NotifyCenter.cpp
	    BufferedDrawTarget.cpp
            CompositeDrawContext.cpp
	    PosixPreferencesManager.cpp
            SlaveEventManager.cpp
            rgba.cpp
            DataPathManager.cpp
           )

TARGET_INCLUDE_DIRECTORIES(gametools PRIVATE
                           ${CMAKE_SOURCE_DIR}/iosfc
                          )

IF (ENABLE_NULL_BACKENDS)

ENDIF (ENABLE_NULL_BACKENDS)

IF (ENABLE_SDL12_BACKENDS)
ADD_SUBDIRECTORY(sdl_drawcontext)
ENDIF (ENABLE_SDL12_BACKENDS)

IF (ENABLE_SDL2_BACKENDS)
ADD_SUBDIRECTORY(sdl_drawcontext)
ENDIF (ENABLE_SDL2_BACKENDS)
