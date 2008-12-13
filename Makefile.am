ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS=flobopuyo
flobopuyo_SOURCES=audio.cpp HiScores.cpp PuyoCommander.cpp \
          MainMenu.cpp MainScreen.cpp \
          main.cpp PuyoDataPathManager.cpp PuyoGame.cpp PuyoDoomMelt.cpp \
          PuyoView.cpp PuyoAnimations.cpp AnimatedPuyo.cpp PuyoIA.cpp \
          styrolyse.cpp PuyoStory.cpp PuyoChatBox.cpp PuyoLocalizedDictionary.cpp \
          PuyoEventPlayer.cpp PuyoPauseMenu.cpp PuyoCheatCodeManager.cpp \
          PuyoGameWidget.cpp PuyoStatsWidget.cpp \
          PuyoStarter.cpp PuyoSinglePlayerStarter.cpp PuyoTwoPlayerStarter.cpp \
          PuyoNetworkStarter.cpp PuyoNetworkView.cpp PuyoNetworkGame.cpp \
          AnimatedPuyoTheme.cpp PuyoNetworkMenu.cpp PuyoInternetMenu.cpp PuyoNetCenterMenu.cpp \
          PuyoNetGameCenter.cpp PuyoInternetGameCenter.cpp PuyoLanGameCenter.cpp\
          PuyoLocalMenu.cpp PuyoOptionMenu.cpp PuyoControlMenu.cpp \
          LevelThemeMenu.cpp PuyoThemeMenu.cpp PuyoScreenTransition.cpp \
          PuyoNatTraversal.cpp PuyoGameAbstract.cpp PuyoStrings.cpp \
          PuyoMessageDef.cpp
if MACOSX
flobopuyo_SOURCES+=SDLMain.m
endif

EXTRA_DIST=PuyoEventPlayer.h PuyoNetworkView.h \
           AnimatedPuyo.h PuyoGameAbstract.h PuyoOptionMenu.h \
           AnimatedPuyoTheme.h PuyoGame.h PuyoPauseMenu.h \
           audio.h PuyoGameWidget.h PuyoPlayer.h \
           PuyoIA.h PuyoScoring.h \
           glSDL.h PuyoIgpDefs.h PuyoScreenTransition.h \
           HallOfFame.h PuyoInternetGameCenter.h PuyoSinglePlayerStarter.h \
           HiScores.h PuyoInternetMenu.h PuyoStarter.h \
           LevelThemeMenu.h PuyoLanGameCenter.h PuyoStatsWidget.h \
           MainMenu.h PuyoLocalizedDictionary.h  PuyoStory.h \
           MainScreen.h PuyoLocalMenu.h PuyoStrings.h \
           PuyoAnimations.h PuyoMessageDef.h PuyoThemeMenu.h \
           PuyoChatBox.h PuyoNatTraversal.h PuyoTwoPlayerStarter.h \
           PuyoCheatCodeManager.h PuyoNetCenterMenu.h PuyoView.h \
           PuyoCommander.h PuyoNetGameCenter.h SDLMain.h \
           PuyoControlMenu.h PuyoNetworkGame.h styrolyse.h \
           PuyoDataPathManager.h PuyoNetworkMenu.h \
           PuyoDoomMelt.h PuyoNetworkStarter.h

flobopuyo_CXXFLAGS = -I$(top_srcdir)/iosfc -I$(top_srcdir)/gametools -I$(top_srcdir)/goomsl $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)
flobopuyo_OBJCFLAGS= $(SDL_CFLAGS)
flobopuyo_LDADD = -Lgametools -Lgoomsl -Liosfc -lgametools -lgoomsl -liosfc $(SDL_LIBS)

SUBDIRS = iosfc gametools goomsl data
