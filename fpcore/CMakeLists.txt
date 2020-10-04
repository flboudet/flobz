cmake_minimum_required(VERSION 2.8)

ADD_LIBRARY(fpcore STATIC
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


TARGET_INCLUDE_DIRECTORIES(fpcore PRIVATE
                           ${CMAKE_SOURCE_DIR}/iosfc
                           ${CMAKE_SOURCE_DIR}/gametools
                           ${CMAKE_SOURCE_DIR}/goomsl
                          )

TARGET_COMPILE_DEFINITIONS(fpcore PRIVATE
                           SDL12_GFX)
