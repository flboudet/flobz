#include ../root_dir
#include ../config

SRCS=ios_exception.cpp ios_stream.cpp ios_hash.cpp ios_memory.cpp ios_message.cpp ios_standardmessage.cpp ios_udpmessage.cpp ios_messagebox.cpp ios_udpmessagebox.cpp  ios_datagramsocket.cpp ios_unixdatagramsocketimpl.cpp ios_unixsocketaddressimpl.cpp ios_igpmessagebox.cpp ios_socket.cpp ios_unixsocketimpl.cpp ios_igpclient.cpp ios_igpdatagram.cpp ios_filemanager.cpp ios_time.cpp

OBJECTS=${SRCS:%.cpp=%.o}

all: ${OBJECTS}
	ar -cru iosfc.la ${OBJECTS}

object: ${OBJECTS}

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = $(CXX) -MM $(CFLAGS_NOPCH) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CXXFLAGS) -f $*.d $<

%.o:%.cpp
	@mkdir -p $(DEPDIR); \
	$(MAKEDEPEND);  \
	cp $(df).d $(df).P;  \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

clean:
	rm -f *.o *~

.PHONY: all object clean

-include $(SRCS:%.cpp=$(DEPDIR)/%.P)
# DO NOT DELETE
