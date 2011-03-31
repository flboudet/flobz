noinst_LTLIBRARIES=libfpcore.la

libfpcore_la_SOURCES=audio.cpp HiScores.cpp PuyoCommander.cpp \
          MainMenu.cpp MainScreen.cpp PuyoMain.cpp \
          main.cpp FPDataPathManager.cpp PuyoGame.cpp DoomMelt.cpp \
          PuyoView.cpp PuyoAnimations.cpp AnimatedPuyo.cpp AIPlayer.cpp \
          styrolyse.cpp PuyoStory.cpp ChatBox.cpp LocalizedDictionary.cpp \
          PuyoEventPlayer.cpp PauseMenu.cpp CheatCodeManager.cpp \
          GameWidget.cpp GameScreen.cpp StatsWidget.cpp \
          LocalGameStates.cpp \
          PuyoStarter.cpp PuyoSinglePlayerStarter.cpp PuyoTwoPlayerStarter.cpp \
          LocalMenu.cpp OptionMenu.cpp ControlMenu.cpp \
          LevelThemeMenu.cpp ThemeMenu.cpp ScreenTransition.cpp \
          PuyoGameAbstract.cpp PuyoStrings.cpp \
          PackageDescription.cpp AnimatedPuyoTheme.cpp

# Optional sources for network gaming support
if ENABLE_NETWORK
libfpcore_la_SOURCES+=PuyoNetworkStarter.cpp PuyoNetworkView.cpp PuyoNetworkGame.cpp \
          NetworkMenu.cpp InternetMenu.cpp PuyoInternetBot.cpp NetCenterMenu.cpp \
          NetGameCenter.cpp InternetGameCenter.cpp LanGameCenter.cpp\
          NatTraversal.cpp PuyoMessageDef.cpp
endif

libfpcore_la_CXXFLAGS = -I$(top_srcdir)/iosfc -I$(top_srcdir)/goomsl -I$(top_srcdir)/gametools $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=PuyoEventPlayer.h PuyoNetworkView.h \
           AnimatedPuyo.h PuyoGameAbstract.h OptionMenu.h \
           PuyoGame.h PauseMenu.h \
           audio.h GameWidget.h GameScreen.h PuyoPlayer.h \
           AIPlayer.h PuyoScoring.h PuyoMain.h \
           ScreenTransition.h \
           HallOfFame.h InternetGameCenter.h PuyoSinglePlayerStarter.h \
           HiScores.h InternetMenu.h PuyoInternetBot.h PuyoStarter.h \
           LevelThemeMenu.h LanGameCenter.h StatsWidget.h \
           MainMenu.h LocalizedDictionary.h  PuyoStory.h \
           MainScreen.h LocalMenu.h PuyoStrings.h \
           PuyoAnimations.h PuyoMessageDef.h ThemeMenu.h \
           ChatBox.h NatTraversal.h LocalGameStates.h \
           PuyoTwoPlayerStarter.h \
           CheatCodeManager.h NetCenterMenu.h PuyoView.h \
           PuyoCommander.h NetworkDefinitions.h NetGameCenter.h SDLMain.h \
           ControlMenu.h PuyoNetworkGame.h styrolyse.h \
           FPDataPathManager.h NetworkMenu.h \
           DoomMelt.h PuyoNetworkStarter.h PackageDescription.h \
           Theme.h

