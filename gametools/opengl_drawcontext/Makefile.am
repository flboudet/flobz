if ENABLE_OPENGL_GFX

noinst_LTLIBRARIES=libopengl_drawcontext.la

libopengl_drawcontext_la_SOURCES=OpenGL_IosFont.cpp \
          OpenGL_AudioManager.cpp \
          OpenGL_drawcontext.cpp OpenGL_eventmanager.cpp

libopengl_drawcontext_la_CXXFLAGS = -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc $(AM_CXXFLAGS) $(FPFEATURES)

endif
