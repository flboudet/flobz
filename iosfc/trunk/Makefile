OBJ=ios_exception.o ios_hash.o ios_memory.o ios_message.o ios_udpmessage.o ios_messagebox.o ios_udpmessagebox.o  ios_vector.o IosDatagramSocket.o
TOBJ=test_udpmessagebox.o
CPPFLAGS=-g `sdl-config --cflags`  -Wall

all:${OBJ} ${TOBJ}
	g++ `sdl-config --cflags` -o test ${OBJ} ${TOBJ} `sdl-config --libs` -lSDL_net

object:${OBJ}

clean:
	rm -f *.o *~
