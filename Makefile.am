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

# Mac OS X Targets

bundle_name = $(abs_top_builddir)/FloboPuyo.app
macimage_name = $(abs_top_builddir)/FloboPuyo_Image

mac-bundle:
	rm -Rf "$(bundle_name)"
	mkdir -p "$(bundle_name)/Contents/MacOS"
	mkdir -p "$(bundle_name)/Contents/Resources"
	mkdir -p "$(bundle_name)/Contents/Frameworks"
	echo "APPL????" > "$(bundle_name)/Contents/PkgInfo"
	sed "s/@@VERSION@@/$(VERSION)/" mac/Info.plist > "$(bundle_name)/Contents/Info.plist"
	cp mac/icon.icns "$(bundle_name)/Contents/Resources/"
	cp $(bin_PROGRAMS) "$(bundle_name)/Contents/MacOS/"
	$(MAKE) $(AM_MAKEFLAGS) install fpdata="$(bundle_name)/Contents/Resources/data" datarootdir="$(bundle_name)/Contents/Resources" bindir="$(windistdir)"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_image.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_mixer.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_ttf.framework" "$(bundle_name)/Contents/Frameworks"

mac-package: mac-bundle
	rm -rf $(macimage_name)
	rm -f $(macimage_name).dmg
	rm -f $(macimage_name).tmp.dmg
	mkdir -p $(macimage_name)
	cp -RP $(bundle_name) $(macimage_name)
	cp COPYING $(macimage_name)
	hdiutil create -srcfolder $(macimage_name) $(macimage_name).tmp.dmg
	hdiutil convert -format UDZO -imagekey zlib-level=9 -o $(macimage_name).dmg $(macimage_name).tmp.dmg
	hdiutil internet-enable $(macimage_name).dmg
	rm -rf $(macimage_name)
	rm -f $(macimage_name).tmp.dmg

# Windows targets

windistdir = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32"
wininstaller = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-setup.exe"
winziparchive = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32.zip"
win_dll_to_copy=jpeg.dll libpng12-0.dll SDL.dll SDL_mixer.dll zlib1.dll \
libfreetype-6.dll libtiff-3.dll SDL_image.dll SDL_ttf.dll

win_binary_distribution:
	rm -rf $(windistdir)
	mkdir $(windistdir)
	$(MAKE) $(AM_MAKEFLAGS) install fpdata="$(windistdir)/data" datarootdir="$(windistdir)" bindir="$(windistdir)"
	cp COPYING $(windistdir)
	for curlib in $(win_dll_to_copy); do find `dirname \`dirname "$(SDL_CONFIG)"\`` -name "$${curlib}" -exec cp {} $(windistdir) \; ; done

win_installer: win_binary_distribution
	cd $(windistdir) \
	&& makensis -NOCD $(abs_top_srcdir)/win32/installer.nsi \
	&& mv FloboPuyoSetup.exe $(wininstaller)

win_zip_archive: win_binary_distribution
	rm -f $(winziparchive)
	cd `dirname $(windistdir)` \
	&& zip -9 -r $(winziparchive) `basename $(windistdir)`


