# Makefile
# author: iOS-Software
# July 2004

###############
# Settings

ENABLE_AUDIO=true
ENABLE_OPENGL=false
ENABLE_DGA=false

DEBUG_MODE=true
MORE_DEBUG=false

# Unix/Linux settings
PREFIX=/usr/local
DATADIR=$(PREFIX)/share/games/flobopuyo
INSTALL_BINDIR=$(DESTDIR)/$(PREFIX)/games
INSTALL_DATADIR=$(DESTDIR)/$(DATADIR)
IOSFC_DIR=iosfc

# Mac settings
macimage_name=FloBoPuyo\ $(VERSION)
bundle_name=FloboPuyo.app

# Win32 settings
CYGWIN_VERSION=CYGWIN_NT-5.1
WINZIP_NAME=flobopuyowin32
WINSDLINCLUDE=/home/$(USER)/SDL-1.2.7/include
WINSDLDEVLIBS=/home/$(USER)/SDL-1.2.7/lib
WINSDLRUNTIME=/home/$(USER)/SDL-1.2.7/lib
#
##########


##############
# Autodetection

ifndef PLATFORM
PLATFORM=$(shell uname -s)
endif


VERSION=$(shell grep "\#define FLOBOPUYOVERSION" PuyoVersion.c  | cut -d "\"" -f 2)
#
##############

SDL_CONFIG=sdl-config
CC=g++
CXX=g++

CFLAGS= -DDATADIR=\"${DATADIR}\" -I${IOSFC_DIR}
LDFLAGS=

HFILES= HiScores.h IosException.h IosImgProcess.h IosVector.h PuyoCommander.h\
        PuyoGame.h PuyoAnimations.h AnimatedPuyo.h PuyoIA.h PuyoPlayer.h     \
        PuyoStory.h PuyoView.h SDL_prim.h audio.h menu.h menuitems.h         \
        preferences.h scrollingtext.h sofont.h SDL_Painter.h PuyoVersion.h   \
        InputManager.h GameControls.h HiScores.h  PuyoNetworkView.h          \
        IosImgProcess.h PuyoStarter.h PuyoNetworkGame.h


OBJFILES= SDL_prim.o HiScores.o scenar.y.o scenar.l.o PuyoCommander.o        \
          IosException.o IosVector.o main.o PuyoGame.o PuyoVersion.o         \
          PuyoView.o PuyoAnimations.o AnimatedPuyo.o PuyoIA.o sofont.o       \
          menu.o menuitems.o audio.o scrollingtext.o preferences.o           \
          PuyoStory.o SDL_Painter.o InputManager.o GameControls.o            \
          PuyoDoomMelt.o IosImgProcess.o corona32.o corona.o corona_palette.o\
	  PuyoStarter.o PuyoNetworkView.o PuyoNetworkGame.o


# Autodetected cygwin platform
ifeq ($(PLATFORM), $(CYGWIN_VERSION))
  CFLAGS:=$(CFLAGS) -mno-cygwin -mwindows -DWIN32 -DYY_NEVER_INTERACTIVE=1 -I$(WINSDLINCLUDE)
  LDFLAGS:=$(LDFLAGS) -L$(WINSDLDEVLIBS) -lmingw32 -ljpeg -lzlib -lpng1 -lSDL_net -lSDL_image -lSDL_mixer -lSDL -lSDLmain 
else
# mingw32 cross-compilation (works great in debian testing)
ifeq ($(PLATFORM), crossmingw32)
  CC=i586-mingw32msvc-g++
  CXX=i586-mingw32msvc-g++
  CFLAGS:=$(CFLAGS) -mno-cygwin -mwindows -DWIN32 -DYY_NEVER_INTERACTIVE=1 -I$(WINSDLINCLUDE)  -I$(WINSDLINCLUDE)/SDL
  LDFLAGS:=$(LDFLAGS) -L$(WINSDLDEVLIBS) -lmingw32 -ljpeg -lzlib -lpng1 -lSDL_net -lSDL_image -lSDL_mixer -lSDL -lSDLmain 
else
# more traditional unix platform
CFLAGS:=$(CFLAGS) `$(SDL_CONFIG) --cflags`
LDFLAGS:=$(LDFLAGS) `$(SDL_CONFIG) --cflags --libs`
#small additions for Darwin (we assume SDL was installed with Fink)
ifeq ($(PLATFORM), Darwin)
CFLAGS:=$(CFLAGS) -I/sw/include -DMACOSX -UDATADIR
endif
endif
endif


ifeq ($(ENABLE_AUDIO), true)
CFLAGS:=$(CFLAGS) -DUSE_AUDIO=1
OBJFILES:=$(OBJFILES)
endif

ifeq ($(ENABLE_OPENGL), true)
CFLAGS:=$(CFLAGS) -DHAVE_OPENGL=1
OBJFILES:=$(OBJFILES) glSDL.o
ifeq ($(PLATFORM), Linux)
LDFLAGS:=$(LDFLAGS) -lGL
endif
endif

ifeq ($(ENABLE_DGA), true)
ifeq ($(PLATFORM), Linux)
CFLAGS:=$(CFLAGS) -DUSE_DGA=1
endif
endif

ifeq ($(DEBUG_MODE), true)
CFLAGS:=$(CFLAGS) -g
LDFLAGS:=$(LDFLAGS) -g
endif

ifeq ($(MORE_DEBUG), true)
CFLAGS:=$(CFLAGS) -DDEBUG=1
endif

CXXFLAGS=${CFLAGS}

all: prelude flobopuyo

flobopuyo: ${OBJFILES}
	@make -C iosfc object
	@echo "[flobopuyo]" && $(CXX) $(CFLAGS) $(LDFLAGS) -o flobopuyo -lSDL_net -lSDL_mixer -lSDL_image ${OBJFILES} iosfc/*.o
	@echo "--------------------------------------"
	@echo " Compilation finished"
	@[ "x`cat WARNINGS | wc -l`" != "x0" ] && echo -e "--------------------------------------\n There have been some warnings:\n" && cat WARNINGS && rm -f WARNINGS && echo "--------------------------------------" || true
	@echo
	@echo " Type ./flobopuyo to play."
	@echo "--------------------------------------"

prelude:
	@rm -f WARNINGS
	@touch WARNINGS
	@echo "Compiling with CFLAGS=$(CFLAGS)"
	@echo "Compiling with LDFLAGS=$(LDFLAGS)"

%.o:%.c
	@echo "[$@]" && $(CC) $(CFLAGS) -c $< 2>> WARNINGS || (cat WARNINGS && false)

%.o:%.cpp
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< 2>> WARNINGS || (cat WARNINGS && false)

PuyoDoomMelt.o:PuyoDoomMelt.c ${HFILES}
HiScores.o:HiScores.cpp HiScores.h preferences.h
PuyoCommander.o:PuyoCommander.cpp ${HFILES}
PuyoGame.o:PuyoGame.cpp ${HFILES}
PuyoIA.o:PuyoIA.cpp ${HFILES}
PuyoStory.o:PuyoStory.cpp ${HFILES}
PuyoView.o:PuyoView.cpp ${HFILES}
AnimatedPuyo.o: AnimatedPuyo.cpp ${HFILES}
PuyoAnimations.o: PuyoAnimations.cpp ${HFILES}
main.o:main.cpp ${HFILES}
preferences.o:preferences.c preferences.h
scenar.l.o:scenar.l.c ${HFILES}
scenar.y.o:scenar.y.c ${HFILES}
InputManager.o:InputManager.cpp ${HFILES}
GameControls.o:GameControls.cpp ${HFILES}
IosImgProcess.o:IosImgProcess.cpp ${HFILES}
SDL_Painter.o:SDL_Painter.cpp SDL_Painter.h
audio.o:audio.c audio.h
menu.o:menu.c menu.h menuitems.h
menuitems.o:menuitems.c menu.h menuitems.h
scrollingtext.o:scrollingtext.c
sofont.o:sofont.c
IosException.o:IosException.cpp
IosVector.o:IosVector.cpp
glSDL.o:glSDL.c
	@echo "[$@]" && $(CC) $(CFLAGS) -c $< 2>> EXT_WARNINGS
	@rm -f EXT_WARNINGS
SDL_prim.o:SDL_prim.c
	@echo "[$@]" && $(CC) $(CFLAGS) -c $< 2>> EXT_WARNINGS
	@rm -f EXT_WARNINGS
corona.o:corona.cpp
corona32.o:corona32.cpp
corona_palette.o:corona_palette.cpp	

scenar.l.c:scenar.l ${HFILES}
	@echo "[$@]" && flex -oscenar.l.c scenar.l
scenar.y.c:scenar.y ${HFILES}
	@echo "[$@]" && bison -y -d -o scenar.y.c scenar.y

clean:
	make -C iosfc clean
	rm -rf *~ scenar.y.c scenar.y.h scenar.l.c *.o flobopuyo* WARNINGS
	rm -rf .xvpics data/.xvpics    data/*/.xvpics
	rm -rf $(bundle_name)
	rm -rf $(macimage_name)
	rm -f  $(macimage_name).dmg
	rm -f  .DS_Store */.DS_Store */*/.DS_Store .gdb_history

install: flobopuyo
	strip flobopuyo
	mkdir -p ${INSTALL_BINDIR}
	mkdir -p ${INSTALL_DATADIR}
	cp -r data/* ${INSTALL_DATADIR}
	chmod a+rx ${INSTALL_DATADIR}
	chmod a+rx ${INSTALL_DATADIR}/sfx
	chmod a+rx ${INSTALL_DATADIR}/gfx
	chmod a+rx ${INSTALL_DATADIR}/story
	chmod -R a+r ${INSTALL_DATADIR}
	cp ./flobopuyo ${INSTALL_BINDIR}/flobopuyo
	chmod a+rx ${INSTALL_BINDIR}/flobopuyo

flobopuyo-static: prelude  ${OBJFILES}
	@echo "[flobopuyo-static]" && g++ $(CFLAGS) -o flobopuyo-static ${OBJFILES} iosfc/*.o\
        /sw/lib/libSDL_mixer.a /sw/lib/libSDL_net.a /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /sw/lib/libsmpeg.a /sw/lib/libSDL_image.a /sw/lib/libjpeg.a /sw/lib/libpng.a -lz `$(SDL_CONFIG) --static-libs`
	@echo "--------------------------------------"
	@echo " Compilation finished"

bundle: flobopuyo-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	sed "s/@@VERSION@@/$(VERSION)/" mac/Info.plist > $(bundle_name)/Contents/Info.plist
	cp mac/icon.icns $(bundle_name)/Contents/Resources/
	cp flobopuyo-static $(bundle_name)/Contents/MacOS/flobopuyo
	cp -r data $(bundle_name)/Contents/Resources
	rm -rf $(bundle_name)/Contents/Resources/data/CVS $(bundle_name)/Contents/Resources/data/*/CVS
	rm -rf $(bundle_name)/Contents/Resources/data/.xvpics $(bundle_name)/Contents/Resources/data/*/.xvpics
	rm -f $(bundle_name)/Contents/Resources/data/.DS_Store $(bundle_name)/Contents/Resources/data/*/.DS_Store
	strip $(bundle_name)/Contents/MacOS/flobopuyo

mac-package: bundle
	mkdir -p $(macimage_name)
	cp -r $(bundle_name) $(macimage_name)
	cp COPYING $(macimage_name)
	hdiutil create -srcfolder $(macimage_name) $(macimage_name).dmg
	hdiutil internet-enable $(macimage_name).dmg

win-package: flobopuyo
	mkdir -p $(WINZIP_NAME)
	cp -r data $(WINZIP_NAME)
	cp flobopuyo.exe $(WINZIP_NAME)
	cp COPYING $(WINZIP_NAME)
	cp $(WINSDLRUNTIME)/*.dll $(WINZIP_NAME)
	zip -r $(WINZIP_NAME) $(WINZIP_NAME)
