# Makefile
# author: iOS-Software
# July 2004

#BINDIR=/usr/games/
DATADIR=\"data\"

# SDL_CONFIG=/home/jeko/opt/bin/sdl-config
SDL_CONFIG=sdl-config
CC=g++
CXX=g++

CFLAGS=`$(SDL_CONFIG) --cflags` -O3 -I/sw/include -DUSE_AUDIO
CXXFLAGS=${CFLAGS}

HFILES= IosException.h IosImgProcess.h IosVector.h PuyoCommander.h PuyoGame.h \
        PuyoIA.h PuyoPlayer.h PuyoStory.h PuyoView.h SDL_prim.h audio.h menu.h \
				menuitems.h preferences.h scrollingtext.h sofont.h SDL_Painter.h

OBJFILES= SDL_prim.o scenar.y.o scenar.l.o PuyoCommander.o IosException.o \
					IosVector.o main.o PuyoGame.o PuyoView.o PuyoIA.o sofont.o \
					menu.o menuitems.o audio.o scrollingtext.o preferences.o PuyoStory.o SDL_Painter.o

all: prelude ${OBJFILES}
	@echo "[flobopuyo]" && g++ $(CFLAGS) -o flobopuyo `$(SDL_CONFIG) --cflags --libs` -lSDL_mixer -lSDL_image ${OBJFILES}
	@echo "--------------------------------------"
	@echo " Compilation finished"
	@[ "x`cat WARNINGS | wc -l`" != "x0" ] && echo -e "--------------------------------------\n There have been some warnings:\n" && cat WARNINGS && rm -f WARNINGS && echo "--------------------------------------" || true
	@echo
	@echo " Type ./flobopuyo to play."
	@echo "--------------------------------------"
	
prelude:
	@rm -f WARNINGS
	@touch WARNINGS

%.o:%.c
	@echo "[$@]" && $(CC) $(CFLAGS) -DDATADIR=${DATADIR} -c $< 2>> WARNINGS || (cat WARNINGS && false)
	
%.o:%.cpp
	@echo "[$@]" && $(CXX) $(CFLAGS) -DDATADIR=${DATADIR} -c $< 2>> WARNINGS || (cat WARNINGS && false)

PuyoCommander.o:PuyoCommander.cpp ${HFILES}
PuyoGame.o:PuyoGame.cpp ${HFILES}
PuyoIA.o:PuyoIA.cpp ${HFILES}
PuyoStory.o:PuyoStory.cpp ${HFILES}
PuyoView.o:PuyoView.cpp ${HFILES}
main.o:main.cpp ${HFILES}
preferences.o:preferences.c ${HFILES}
scenar.l.o:scenar.l.c ${HFILES}
scenar.y.o:scenar.y.c ${HFILES}
SDL_Painter.o:SDL_Painter.cpp SDL_Painter.h
audio.o:audio.c audio.h
menu.o:menu.c menu.h menuitems.h
menuitems.o:menuitems.c menu.h menuitems.h
scrollingtext.o:scrollingtext.c
sofont.o:sofont.c
SDL_prim.o:SDL_prim.c
IosException.o:IosException.cpp
IosVector.o:IosVector.cpp

scenar.l.c:scenar.l ${HFILES}
	@echo "[$@]" && flex -oscenar.l.c scenar.l
scenar.y.c:scenar.y ${HFILES}
	@echo "[$@]" && bison -y -d -o scenar.y.c scenar.y

clean:
	rm -f *~ scenar.y.c scenar.y.h scenar.l.c *.o flobopuyo* WARNINGS

#_install: ${OBJFILES}
#	g++ $(CFLAGS) -o flobopuyo `$(SDL_CONFIG) --cflags --static-libs` -lSDL_mixer -lSDL_image ${OBJFILES}

#install: _install
#	strip --strip-all flobopuyo
#	mkdir -p ${DATADIR}
#	cp -r data/* ${DATADIR}
#	chmod a+rx ${DATADIR}
#	chmod a+rx ${DATADIR}/sfx
#	chmod a+rx ${DATADIR}/gfx
#	chmod a+rx ${DATADIR}/story
#	chmod -R a+r ${DATADIR}
#	cp ./flobopuyo ${BINDIR}/flobopuyo
#	chmod a+rx ${BINDIR}/flobopuyo

bundle_name = FloboPuyo.app
flobopuyo-static:SDL_prim.o
	bison -y -d -o scenar.y.c scenar.y
	flex -oscenar.l.c scenar.l
	g++ -DMACOSX $(CFLAGS) -o flobopuyo-static  SDL_prim.o PuyoCommander.cpp IosException.cpp IosVector.cpp main.cpp PuyoGame.cpp PuyoView.cpp PuyoIA.cpp sofont.c menu.c menuitems.c audio.c scrollingtext.c preferences.c SDL_Painter.cpp PuyoStory.cpp scenar.y.c scenar.l.c /sw/lib/libSDL_mixer.a /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /sw/lib/libsmpeg.a /sw/lib/libSDL_image.a /sw/lib/libjpeg.a /sw/lib/libpng.a -lz `$(SDL_CONFIG) --cflags --static-libs`

# /sw/lib/libvorbis.a   

bundle: flobopuyo-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp mac/Info.plist $(bundle_name)/Contents/
	cp mac/icon.icns $(bundle_name)/Contents/Resources/
	cp flobopuyo-static $(bundle_name)/Contents/MacOS/flobopuyo
	cp -r data $(bundle_name)/Contents/Resources
	strip $(bundle_name)/Contents/MacOS/flobopuyo
