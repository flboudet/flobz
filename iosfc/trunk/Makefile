OBJ=ios_exception.o ios_stream.o ios_hash.o ios_memory.o ios_message.o ios_standardmessage.o ios_udpmessage.o ios_messagebox.o ios_udpmessagebox.o  IosDatagramSocket.o ios_igpmessagebox.o ios_socket.o ios_unixsocketimpl.o ios_igpclient.o ios_igpdatagram.o
TOBJ=test_udpmessagebox.o
CPPFLAGS=-g `sdl-config --cflags`  -Wall

# mingw32 cross-compilation (works great in debian testing)
ifeq ($(PLATFORM), crossmingw32)
  CC=i586-mingw32msvc-g++
  CXX=i586-mingw32msvc-g++
  WINSDLINCLUDE=/home/$(USER)/SDL-1.2.7/include
  CPPFLAGS=-g -mno-cygwin -mwindows -DWIN32 -DYY_NEVER_INTERACTIVE=1 -I$(WINSDLINCLUDE)  -I$(WINSDLINCLUDE)/SDL  -Wall
endif

all:${OBJ} ${TOBJ}
	$(CXX) `sdl-config --cflags` -o test ${OBJ} ${TOBJ} `sdl-config --libs` -lSDL_net

object:${OBJ}

clean:
	rm -f *.o *~
