OBJ=ios_exception.o ios_hash.o ios_memory.o ios_message.o ios_udpmessage.o ios_messagebox.o ios_udpmessagebox.o  ios_vector.o IosDatagramSocket.o test_udpmessagebox.o
CPPFLAGS=-g `sdl-config --cflags`  -Wall

all:${OBJ}
	g++ `sdl-config --cflags` -o test ${OBJ} `sdl-config --libs` -lSDL_net

clean:
	rm -f *.o *~
