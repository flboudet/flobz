cmake_minimum_required(VERSION 3.0)

add_library(fpcore STATIC
            audio.cpp
            HiScores.cpp
            FPResources.cpp
            FPCommander.cpp
            MainMenu.cpp
            MainScreen.cpp
            FPMain.cpp
            FPDataPathManager.cpp
            FloboGame.cpp
            DoomMelt.cpp
            GameView.cpp
            FloboAnimations.cpp
            AnimatedFlobo.cpp
            AIPlayer.cpp
            styrolyse.cpp
            Story.cpp
            ChatBox.cpp
            LocalizedDictionary.cpp
            EventPlayer.cpp
            PauseMenu.cpp
            CheatCodeManager.cpp
            GameWidget.cpp
            GameScreen.cpp
            StatsWidget.cpp
            LocalGameStates.cpp
            SoloGameStarter.cpp
            StoryModeStarter.cpp
            TwoPlayersGameStarter.cpp
            LocalMenu.cpp
            OptionMenu.cpp
            ControlMenu.cpp
            LevelThemeMenu.cpp
            ThemeMenu.cpp
            ScreenTransition.cpp
            FloboGameAbstract.cpp
            FPStrings.cpp
            PackageDescription.cpp
            AnimatedFloboTheme.cpp
            GSLFileAccessWrapper.cpp
	    PlayerNameUtils.cpp
            HallOfFame.cpp
            ZoomScreenTransition.cpp
           )

target_include_directories(fpcore PRIVATE
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${CMAKE_SOURCE_DIR}/goomsl
                          )

if (ENABLE_NULL_BACKENDS)
target_compile_definitions(fpcore PRIVATE
                           NULL_BACKENDS)
endif (ENABLE_NULL_BACKENDS)

if (ENABLE_SDL12_BACKENDS)

target_compile_definitions(fpcore PRIVATE
                           SDL12_GFX)

# TODO: avoid exposing too much of SDL
target_include_directories(fpcore PRIVATE
                           ${SDL_INCLUDE_DIR}
)

target_compile_definitions(fpcore PRIVATE
                           HAVE_SDL_SDL_MIXER_H
)
endif (ENABLE_SDL12_BACKENDS)

if (ENABLE_SDL2_BACKENDS)

target_compile_definitions(fpcore PRIVATE
                           SDL2_GFX)

# TODO: avoid exposing too much of SDL
target_include_directories(fpcore PRIVATE
                           ${SDL2_INCLUDE_DIRS}
)

target_compile_definitions(fpcore PRIVATE
                           HAVE_SDL_SDL_MIXER_H
)
endif (ENABLE_SDL2_BACKENDS)
