noinst_LTLIBRARIES=libfpcore.la

libfpcore_la_SOURCES=audio.cpp HiScores.cpp FPCommander.cpp \
          MainMenu.cpp MainScreen.cpp PuyoMain.cpp \
          main.cpp FPDataPathManager.cpp FloboGame.cpp DoomMelt.cpp \
          GameView.cpp FloboAnimations.cpp AnimatedFlobo.cpp AIPlayer.cpp \
          styrolyse.cpp Story.cpp ChatBox.cpp LocalizedDictionary.cpp \
          EventPlayer.cpp PauseMenu.cpp CheatCodeManager.cpp \
          GameWidget.cpp GameScreen.cpp StatsWidget.cpp \
          LocalGameStates.cpp \
          SoloGameStarter.cpp StoryModeStarter.cpp TwoPlayersGameStarter.cpp \
          LocalMenu.cpp OptionMenu.cpp ControlMenu.cpp \
          LevelThemeMenu.cpp ThemeMenu.cpp ScreenTransition.cpp \
          FloboGameAbstract.cpp PuyoStrings.cpp \
          PackageDescription.cpp AnimatedFloboTheme.cpp GSLFileAccessWrapper.cpp \
	  PlayerNameUtils.cpp

# Optional sources for network gaming support
if ENABLE_NETWORK
libfpcore_la_SOURCES+=NetworkGameStarter.cpp NetworkGameView.cpp NetworkGame.cpp \
          NetworkMenu.cpp InternetMenu.cpp InternetBot.cpp NetCenterMenu.cpp \
          NetGameCenter.cpp InternetGameCenter.cpp LanGameCenter.cpp\
          NatTraversal.cpp FPNetMessageDef.cpp
endif

libfpcore_la_CXXFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/iosfc -I$(top_srcdir)/goomsl -I$(top_srcdir)/gametools $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=EventPlayer.h NetworkGameView.h \
           AnimatedFlobo.h FloboGameAbstract.h OptionMenu.h \
           FloboGame.h PauseMenu.h \
           audio.h GameWidget.h GameScreen.h Player.h \
           AIPlayer.h PuyoScoring.h PuyoMain.h \
           ScreenTransition.h SoloGameStarter.h \
           HallOfFame.h InternetGameCenter.h StoryModeStarter.h \
           HiScores.h InternetMenu.h InternetBot.h \
           LevelThemeMenu.h LanGameCenter.h StatsWidget.h \
           MainMenu.h LocalizedDictionary.h  Story.h \
           MainScreen.h LocalMenu.h PuyoStrings.h \
           FloboAnimations.h FPNetMessageDef.h ThemeMenu.h \
           ChatBox.h NatTraversal.h LocalGameStates.h \
           TwoPlayersGameStarter.h \
           CheatCodeManager.h NetCenterMenu.h GameView.h \
           FPCommander.h NetworkDefinitions.h NetGameCenter.h \
           ControlMenu.h NetworkGame.h styrolyse.h \
           FPDataPathManager.h NetworkMenu.h \
           DoomMelt.h NetworkGameStarter.h PackageDescription.h \
           Theme.h GSLFileAccessWrapper.h PlayerNameUtils.h

