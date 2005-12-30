SRCS=ios_exception.cpp ios_stream.cpp ios_hash.cpp ios_memory.cpp ios_message.cpp ios_standardmessage.cpp ios_udpmessage.cpp ios_messagebox.cpp ios_udpmessagebox.cpp  ios_datagramsocket.cpp ios_igpmessagebox.cpp ios_igpmessage.cpp ios_socket.cpp ios_igpclient.cpp ios_igpdatagram.cpp ios_filemanager.cpp ios_time.cpp ios_httpdocument.cpp ios_thread.cpp ios_igpmessagelistener.cpp ios_igpvirtualpeermessagebox.cpp ios_selector.cpp

OBJECTS=${SRCS:%.cpp=%.o}

-include Config.mk

ifeq ($(PLATFORMTYPE), WIN32)
  SUBDIRECTORIES=win32
endif

ifeq ($(PLATFORMTYPE), POSIX)
  SUBDIRECTORIES=posix
endif

all: prelude libiosfc.a

libiosfc.a: libiosfc_core.a ${SUBDIRECTORIES}
	@rm -rf libiosfc; \
	mkdir libiosfc; \
	cd libiosfc; \
	${AR} -x ../libiosfc_core.a; \
	for libdir in ${SUBDIRECTORIES}; do \
	  ${AR} -x ../$$libdir/libiosfc_$$libdir.a; \
	done; \
	libtool -static -o ../libiosfc.a *.o; \
	cd ..; \
	rm -rf libiosfc \
	echo $(LIBRARIES)

libiosfc_core.a: ${OBJECTS}
	${AR} -cru libiosfc_core.a ${OBJECTS}

prelude:
	@echo "PLATFORM: $(PLATFORM)"
	@echo "PLATFORMTYPE: $(PLATFORMTYPE)"

$(SUBDIRECTORIES):
	@+CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' make -C $@

.PHONY: all prelude ${SUBDIRECTORIES}

-include Rules.mk
