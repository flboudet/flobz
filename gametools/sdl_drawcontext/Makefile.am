if ENABLE_SDL12_GFX

noinst_LTLIBRARIES=libsdl12_drawcontext.la

libsdl12_drawcontext_la_SOURCES=common/SDL_IosFont.cpp \
          sdl12/sdl12_drawcontext.cpp

libsdl12_drawcontext_la_CXXFLAGS = -I$(srcdir)/common -I$(srcdir)/sdl12 -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=common/SDL_IosFont.h sdl12/sdl12_drawcontext.h

endif

if ENABLE_SDL13_GFX

noinst_LTLIBRARIES=libsdl13_drawcontext.la

libsdl13_drawcontext_la_SOURCES=common/SDL_IosFont.cpp \
          sdl13/sdl13_drawcontext.cpp

libsdl13_drawcontext_la_CXXFLAGS = -I$(srcdir)/common -I$(srcdir)/sdl13 -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=common/SDL_IosFont.h sdl13/sdl13_drawcontext.h

endif
