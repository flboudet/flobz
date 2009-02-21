bin_PROGRAMS=fpserver
fpserver_SOURCES=PuyoServerIgpNatTraversal.cpp  PuyoServerMain.cpp \
                 PuyoPeersListenerV1.cpp PuyoPeersListenerV2.cpp

fpserver_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/iosfc
fpserver_LDADD = -L../iosfc -liosfc

EXTRA_DIST=PuyoServerIgpNatTraversal.h PuyoServerIgpResponder.h PuyoPeersListener.h PuyoPeersListenerV1.h PuyoPeersListenerV2.h
