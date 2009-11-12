ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS=flobopuyo
flobopuyo_SOURCES=audio.cpp HiScores.cpp PuyoCommander.cpp \
          MainMenu.cpp MainScreen.cpp PuyoMain.cpp \
          main.cpp DataPathManager.cpp PuyoGame.cpp DoomMelt.cpp \
          PuyoView.cpp PuyoAnimations.cpp AnimatedPuyo.cpp PuyoIA.cpp \
          styrolyse.cpp PuyoStory.cpp ChatBox.cpp LocalizedDictionary.cpp \
          PuyoEventPlayer.cpp PauseMenu.cpp CheatCodeManager.cpp \
          GameWidget.cpp StatsWidget.cpp \
          PuyoStarter.cpp PuyoSinglePlayerStarter.cpp PuyoTwoPlayerStarter.cpp \
          LocalMenu.cpp OptionMenu.cpp ControlMenu.cpp \
          LevelThemeMenu.cpp ThemeMenu.cpp ScreenTransition.cpp \
          PuyoGameAbstract.cpp PuyoStrings.cpp AnimatedPuyoTheme.cpp

# Optional sources for network gaming support
if ENABLE_NETWORK
flobopuyo_SOURCES+=PuyoNetworkStarter.cpp PuyoNetworkView.cpp PuyoNetworkGame.cpp \
          NetworkMenu.cpp InternetMenu.cpp PuyoInternetBot.cpp NetCenterMenu.cpp \
          NetGameCenter.cpp InternetGameCenter.cpp LanGameCenter.cpp\
          NatTraversal.cpp PuyoMessageDef.cpp
endif

# If we are building for Mac OS X, we also need to compile the SDLMain wrapper
if MACOSX
flobopuyo_SOURCES+=SDLMain.m
endif

EXTRA_DIST=PuyoEventPlayer.h PuyoNetworkView.h \
           AnimatedPuyo.h PuyoGameAbstract.h OptionMenu.h \
           AnimatedPuyoTheme.h PuyoGame.h PauseMenu.h \
           audio.h GameWidget.h PuyoPlayer.h \
           PuyoIA.h PuyoScoring.h PuyoMain.h \
           PuyoIgpDefs.h ScreenTransition.h \
           HallOfFame.h InternetGameCenter.h PuyoSinglePlayerStarter.h \
           HiScores.h InternetMenu.h PuyoInternetBot.h PuyoStarter.h \
           LevelThemeMenu.h LanGameCenter.h StatsWidget.h \
           MainMenu.h LocalizedDictionary.h  PuyoStory.h \
           MainScreen.h LocalMenu.h PuyoStrings.h \
           PuyoAnimations.h PuyoMessageDef.h ThemeMenu.h \
           ChatBox.h NatTraversal.h PuyoTwoPlayerStarter.h \
           CheatCodeManager.h NetCenterMenu.h PuyoView.h \
           PuyoCommander.h NetGameCenter.h SDLMain.h \
           ControlMenu.h PuyoNetworkGame.h styrolyse.h \
           DataPathManager.h NetworkMenu.h \
           DoomMelt.h PuyoNetworkStarter.h

flobopuyo_CXXFLAGS = -I$(top_srcdir)/iosfc -I$(top_srcdir)/gametools -I$(top_srcdir)/goomsl $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)
flobopuyo_OBJCFLAGS= $(SDL_CFLAGS)
flobopuyo_LDADD = gametools/libgametools.la goomsl/libgoomsl.la iosfc/libiosfc.la

# Graphic backend configuration
if ENABLE_SDL12_GFX
flobopuyo_LDADD+=gametools/sdl_drawcontext/libsdl12_drawcontext.la
endif

if ENABLE_SDL13_GFX
flobopuyo_LDADD+=gametools/sdl_drawcontext/libsdl13_drawcontext.la
endif

if ENABLE_OPENGL_GFX
flobopuyo_LDADD+=gametools/opengl_drawcontext/libopengl_drawcontext.la
endif

flobopuyo_LDADD+=$(SDL_LIBS)

SUBDIRS = iosfc gametools goomsl data

# Mac OS X Targets

bundle_name = $(abs_top_builddir)/FloboPuyo.app
macimage_name=flobopuyo-$(VERSION)-mac$(REVISION)

mac-bundle:
	rm -Rf "$(bundle_name)"
	mkdir -p "$(bundle_name)/Contents/MacOS"
	mkdir -p "$(bundle_name)/Contents/Resources"
	mkdir -p "$(bundle_name)/Contents/Frameworks"
	echo "APPL????" > "$(bundle_name)/Contents/PkgInfo"
	sed "s/@[@]VERSION[@]@/$(VERSION)$(REVISION)/" $(top_srcdir)/mac/Info.plist > "$(bundle_name)/Contents/Info.plist"
	cp mac/icon.icns "$(bundle_name)/Contents/Resources/"
	cp $(bin_PROGRAMS) "$(bundle_name)/Contents/MacOS/"
	$(MAKE) $(AM_MAKEFLAGS) install fpdata="$(bundle_name)/Contents/Resources/data" datarootdir="$(bundle_name)/Contents/Resources" bindir="$(bundle_name)/Contents/MacOS"
	strip -u -r $(bundle_name)/Contents/MacOS/flobopuyo
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_image.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_mixer.framework" "$(bundle_name)/Contents/Frameworks"
	cp -RP "$(SYSTEM_ROOT)/Library/Frameworks/SDL_ttf.framework" "$(bundle_name)/Contents/Frameworks"
	strip -x $(bundle_name)/Contents/Frameworks/SDL*/SDL*

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

windistdir = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32$(REVISION)"
wininstaller = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)$(REVISION)-setup.exe"
winziparchive = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32$(REVISION).zip"
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

