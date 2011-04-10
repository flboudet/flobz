noinst_LTLIBRARIES=libfpmac.la

libfpmac_la_SOURCES=SDLMain.m
libfpmac_la_OBJCFLAGS= $(SDL_CFLAGS)
if ENABLE_MAC_RECEIPT
libfpmac_la_SOURCES+=validatereceipt.m
endif

EXTRA_DIST=SDLMain.h

