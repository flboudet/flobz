if ENABLE_SDL12_GFX

noinst_LTLIBRARIES=libsdl12_drawcontext.la

libsdl12_drawcontext_la_SOURCES=common/SDL_IosFont.cpp \
          common/SDL_AudioManager.cpp \
          sdl12/sdl12_drawcontext.cpp sdl12/sdl12_eventmanager.cpp

libsdl12_drawcontext_la_CXXFLAGS = -I$(srcdir)/common -I$(srcdir)/sdl12 -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

endif

if ENABLE_SDL13_GFX

noinst_LTLIBRARIES=libsdl13_drawcontext.la

libsdl13_drawcontext_la_SOURCES=common/SDL_IosFont.cpp \
          common/SDL_AudioManager.cpp \
          sdl13/sdl13_drawcontext.cpp sdl13/sdl13_eventmanager.cpp

libsdl13_drawcontext_la_CXXFLAGS = -I$(srcdir)/common -I$(srcdir)/sdl13 -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

endif


EXTRA_DIST=common/SDL_IosFont.h common/SDL_AudioManager.h sdl12/sdl12_drawcontext.h sdl12/sdl12_eventmanager.h sdl13/sdl13_drawcontext.h sdl13/sdl13_eventmanager.h
