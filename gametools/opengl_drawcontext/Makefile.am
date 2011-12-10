if ENABLE_OPENGL_GFX

noinst_LTLIBRARIES=libopengl_drawcontext.la

libopengl_drawcontext_la_SOURCES=OpenGLDrawContext.cpp GLFreeType.cpp StandardTextureLoader.cpp

libopengl_drawcontext_la_CXXFLAGS = -I$(top_srcdir)/gametools -I$(top_srcdir)/iosfc -I/usr/include/freetype2 $(AM_CXXFLAGS) $(FPFEATURES)

endif

EXTRA_DIST=OpenGLDrawContext.h GLFreeType.h StandardTextureLoader.h
