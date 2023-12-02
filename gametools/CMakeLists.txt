cmake_minimum_required(VERSION 3.0)

add_library(gametools STATIC
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

target_include_directories(gametools PRIVATE
                           ${CMAKE_SOURCE_DIR}/iosfc
                          )

if (ENABLE_NULL_BACKENDS)

ENDIF (ENABLE_NULL_BACKENDS)

if (ENABLE_SDL12_BACKENDS)
add_subdirectory(sdl_drawcontext)
endif (ENABLE_SDL12_BACKENDS)

IF (ENABLE_SDL2_BACKENDS)
add_subdirectory(sdl_drawcontext)
endif (ENABLE_SDL2_BACKENDS)
