SRCS=ios_exception.cpp ios_stream.cpp ios_hash.cpp ios_memory.cpp ios_message.cpp ios_standardmessage.cpp ios_udpmessage.cpp ios_messagebox.cpp ios_udpmessagebox.cpp  ios_datagramsocket.cpp ios_unixdatagramsocketimpl.cpp ios_unixsocketaddressimpl.cpp ios_igpmessagebox.cpp ios_socket.cpp ios_unixsocketimpl.cpp ios_igpclient.cpp ios_igpdatagram.cpp ios_filemanager.cpp

CPPFLAGS=-g `sdl-config --cflags`  -Wall
CXX=g++

OBJECTS=${SRCS:%.cpp=%.o}

# mingw32 cross-compilation (works great in debian testing)
ifeq ($(PLATFORM), crossmingw32)
  CXX=i586-mingw32msvc-g++
  CXX=i586-mingw32msvc-g++
  WINSDLINCLUDE=/home/$(USER)/SDL-1.2.7/include
  CPPFLAGS=-g -mno-cygwin -mwindows -DWIN32 -DYY_NEVER_INTERACTIVE=1 -I$(WINSDLINCLUDE)  -I$(WINSDLINCLUDE)/SDL  -Wall
endif

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = gcc -MM $(CPPFLAGS) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CPPFLAGS) -f $*.d $<

all: ${OBJECTS}
	ar -cru iosfc.la ${OBJECTS}

object: ${OBJECTS}

%.o : %.cpp
	@mkdir -p $(DEPDIR);\
	$(MAKEDEPEND); \
	cp $(df).d $(df).P; \
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' < $(df).d >> $(df).P; \
	rm -f $(df).d
	$(COMPILE.cpp) -o $@ $<

clean:
	rm -f *.o *~

.PHONY: all object clean

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
# DO NOT DELETE
