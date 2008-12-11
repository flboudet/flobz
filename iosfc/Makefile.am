noinst_LIBRARIES=libiosfc.a

libiosfc_a_SOURCES=ios_exception.cpp ios_stream.cpp ios_hash.cpp ios_memory.cpp \
            ios_message.cpp ios_standardmessage.cpp ios_udpmessage.cpp ios_messagebox.cpp \
            ios_udpmessagebox.cpp  ios_datagramsocket.cpp ios_igpmessagebox.cpp \
            ios_igpmessage.cpp ios_socket.cpp ios_igpclient.cpp ios_igpdatagram.cpp \
            ios_filemanager.cpp ios_time.cpp ios_httpdocument.cpp ios_thread.cpp \
            ios_igpmessagelistener.cpp ios_igpvirtualpeermessagebox.cpp ios_selector.cpp \
            ios_filepath.cpp ios_networkinterfacerequester.cpp

libiosfc_a_CXXFLAGS = $(AM_CXXFLAGS)

EXTRA_DIST=ios_datagramsocket.h ios_dirigeable.h \
           ios_exception.h ios_fc.h ios_filemanager.h \
           ios_filepath.h ios_filestream.h ios_hash.h \
           ios_httpdocument.h ios_igpclient.h ios_igpdatagram.h \
           ios_igpmessagebox.h ios_igpmessage.h ios_igpmessagelistener.h \
           ios_igpvirtualpeermessagebox.h ios_memory.h ios_messagebox.h \
           ios_message.h ios_networkinterfacerequester.h \
           ios_ptr.h ios_sdlsocketimpl.h ios_selectable.h \
           ios_selector.h ios_server.h ios_serversocket.h \
           ios_socketaddress.h ios_socket.h ios_standardmessage.h \
           ios_stream.h ios_thread.h ios_time.h ios_translator.h \
           ios_udpmessagebox.h ios_udpmessage.h ios_vector.h \
           google/sparse_hash_set google/sparsetable \
           google/dense_hash_map google/type_traits.h \
           google/sparse_hash_map google/dense_hash_set \
           google/sparsehash google/sparsehash/sparseconfig.h \
           google/sparsehash/sparsehashtable.h google/sparsehash/densehashtable.h

include $(srcdir)/posix/Makefile.am
# include $(srcdir)/win32
