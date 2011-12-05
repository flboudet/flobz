noinst_LTLIBRARIES=libgametools.la

libgametools_la_SOURCES=GTLog.cpp GameCursor.cpp \
         gameloop.cpp gameui.cpp GameStateMachine.cpp \
         ListView.cpp Frame.cpp FramedButton.cpp FramedEditField.cpp \
	 mt19937ar.cpp SwitchedButton.cpp RadioButton.cpp NotifyCenter.cpp \
	 BufferedDrawTarget.cpp CompositeDrawContext.cpp \
	 PosixPreferencesManager.cpp SlaveEventManager.cpp

libgametools_la_CXXFLAGS = -I$(top_srcdir)/iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)

EXTRA_DIST=GTLog.h FramedButton.h FramedEditField.h Frame.h \
           GameControls.h GameCursor.h gameloop.h \
           gameui.h GameStateMachine.h DataPathManager.h \
           ListView.h mt19937ar.h NotifyCenter.h \
           RadioButton.h scrollingtext.h \
           SwitchedButton.h vec3.h \
           drawcontext.h audiomanager.h BufferedDrawTarget.h \
           CompositeDrawContext.h PreferencesManager.h \
	   PosixPreferencesManager.h rgba.h ResourceManager.h \
	   ThreadedResourceManager.h NullDrawContext.h NullAudioManager.h \
	   SlaveEventManager.h

SUBDIRS=sdl_drawcontext opengl_drawcontext
