noinst_LTLIBRARIES=libgametools.la

libgametools_la_SOURCES=GameCursor.cpp \
         preferences.cpp gameloop.cpp gameui.cpp \
         ListView.cpp Frame.cpp FramedButton.cpp FramedEditField.cpp \
	 mt19937ar.cpp SwitchedButton.cpp RadioButton.cpp NotifyCenter.cpp \
	 BufferedDrawTarget.cpp CompositeDrawContext.cpp

libgametools_la_CXXFLAGS = -I$(top_srcdir) -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=FramedButton.h FramedEditField.h Frame.h \
           GameControls.h GameCursor.h gameloop.h \
           gameui.h \
           ListView.h mt19937ar.h NotifyCenter.h \
           preferences.h RadioButton.h scrollingtext.h \
           SwitchedButton.h vec3.h \
           drawcontext.h audiomanager.h BufferedDrawTarget.h \
           CompositeDrawContext.h

SUBDIRS=sdl_drawcontext opengl_drawcontext
