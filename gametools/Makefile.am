noinst_LTLIBRARIES=libgametools.la

libgametools_la_SOURCES=SDL_Painter.cpp InputManager.cpp GameControls.cpp GameCursor.cpp \
         IosImgProcess.cpp preferences.cpp sofont.cpp gameloop.cpp gameui.cpp \
         ListView.cpp Frame.cpp FramedButton.cpp FramedEditField.cpp \
	 mt19937ar.cpp SwitchedButton.cpp RadioButton.cpp NotifyCenter.cpp

libgametools_la_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=FramedButton.h FramedEditField.h Frame.h \
           GameControls.h GameCursor.h gameloop.h \
           gameui.h InputManager.h IosImgProcess.h \
           ListView.h mt19937ar.h NotifyCenter.h \
           preferences.h RadioButton.h scrollingtext.h \
           SDL_Painter.h sofont.h SwitchedButton.h vec3.h \
	   drawcontext.h

if ENABLE_SDL12_GFX
libgametools_la_SOURCES+=sdl12_drawcontext.cpp
EXTRA_DIST+=sdl12_drawcontext.h
endif

if ENABLE_SDL13_GFX
libgametools_la_SOURCES+=sdl13_drawcontext.cpp
EXTRA_DIST+=sdl13_drawcontext.h
endif
