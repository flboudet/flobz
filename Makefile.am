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

flobopuyo_SOURCES+=SDLMain.m

flobopuyo_CXXFLAGS = -Iiosfc -Igametools -Igoomsl $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)
flobopuyo_OBJCFLAGS= $(SDL_CFLAGS)
flobopuyo_LDFLAGS = -Lgametools -lgametools -Lgoomsl -lgoomsl -Liosfc -liosfc $(SDL_LIBS)
#                    $(SDL_TTF_LIBS) $(SDL_MIXER_LIBS) $(SDL_IMAGE_LIBS)

SUBDIRS = iosfc gametools goomsl
