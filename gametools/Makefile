ROOT_DIR=..

include ../config

OBJFILES=SDL_Painter.o InputManager.o GameControls.o IosImgProcess.o preferences.o scrollingtext.o sofont.o gameloop.o gameui.o

object:${OBJFILES}

gameloop:
	g++ -Wall ${CFLAGS} `sdl-config --cflags --libs` test_gameloop.cpp gameloop.cpp ../iosfc/*.o -o test_gameloop

gui:object
	g++ -Wall -g -O0 ${CFLAGS} `sdl-config --cflags` test_gui.cpp ../iosfc/*.o ${OBJFILES} -lSDL_image -lSDL_mixer `sdl-config --libs` -o test_gui

gameloop.o: gameloop.cpp gameloop.h
preferences.o:preferences.c preferences.h
InputManager.o:InputManager.cpp   
GameControls.o:GameControls.cpp   
IosImgProcess.o:IosImgProcess.cpp 
scrollingtext.o:scrollingtext.c
sofont.o:sofont.c
SDL_Painter.o:SDL_Painter.cpp SDL_Painter.h
gameui.o:gameui.cpp

clean:
	rm -f *.o *~
