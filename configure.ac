#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#######################################################################
# Configuration                                                       #
#######################################################################

AC_INIT([flobz], [1.0], [flboudet+flobz@gmail.com])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([fpcore/main.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

ENABLE_AUDIO=true
# TODO: remove ENABLE_TTF flag (obsolete)
ENABLE_TTF=true
#ENABLE_BENCHMARKS?=false

# Significant environment variables:
# SDL_CONFIG: path to the sdl-config tool. Ignored if SDL_LIBS is supplied.
# SDL_LIBS
# SDL_CFLAGS
# SDL_VERSION
# FREETYPE_CONFIG: path to the freetype-config tool. Ignored if FREETYPE_LIBS is supplied.
# FREETYPE_LIBS
# FREETYPE_CFLAGS


AC_ARG_ENABLE(network,
       [AS_HELP_STRING([--enable-network], [enables network support])],
       [ENABLE_NETWORK=$enableval], [ENABLE_NETWORK=yes])

AC_ARG_ENABLE(network-internet,
       [AS_HELP_STRING([--enable-network-internet],
                [enables network-internet support])],
       [ENABLE_NETWORK_INTERNET=$enableval], [ENABLE_NETWORK_INTERNET=yes])

AC_ARG_ENABLE(opengl-gfx,
       [AS_HELP_STRING([--enable-opengl-gfx],
                [enables openGL graphics support])],
       [ENABLE_OPENGL_GFX=$enableval], [ENABLE_OPENGL_GFX=no])

AC_ARG_ENABLE(null-backends,
       [AS_HELP_STRING([--enable-null-backends],
                [enables compilation of the game with null backends (no graphics, no sound, no inputs)])],
       [ENABLE_NULL_BACKENDS=$enableval], [ENABLE_NULL_BACKENDS=no])

AC_ARG_ENABLE(emscripten,
       [AS_HELP_STRING([--enable-emscripten],
                [enables compilation of the game with null backends (no graphics, no sound, no inputs)])],
       [ENABLE_EMSCRIPTEN=$enableval], [ENABLE_EMSCRIPTEN=no])

#AC_ARG_ENABLE([audio],
#              [AS_HELP_STRING([--enable-audio],
#              [enable audio support @<:@default=yes@:>@])],
#              [ENABLE_AUDIO=true],
#              [ENABLE_AUDIO=false])

#######################################################################
# Checks for programs.                                                #
#######################################################################
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_OBJC
AM_PROG_AR
AC_PROG_LEX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

case $host_os in
    darwin*)
	    FPCOMMON_FLAGS="$FPCOMMON_FLAGS -DMACOSX"
	    ADDITIONAL_FRAMEWORKS_DIRECTORY="$ADDITIONAL_FRAMEWORKS_DIRECTORY"
	    ADDITIONAL_FRAMEWORKS="$ADDITIONAL_FRAMEWORKS"
	    AC_SUBST([ADDITIONAL_FRAMEWORKS])
	    AC_SUBST([ADDITIONAL_FRAMEWORKS_DIRECTORY])
            ;;
    *)
            ;;
esac

FPCOMMON_FLAGS="$FPCOMMON_FLAGS -pipe"
CXXFLAGS="$CXXFLAGS $FPCOMMON_FLAGS"
CFLAGS="$CFLAGS $FPCOMMON_FLAGS"
OBJCFLAGS="$OBJCFLAGS $FPCOMMON_FLAGS"

# SDL is always required
if test "x$SDL_LIBS" = "x"; then
   # Check for SDL_CONFIG
   if test "x$SDL_CONFIG" = "x"; then
   case $host_os in
   #     darwin*)
   #         SDL_CONFIG=none
   #         ;;
     	 *)
	     AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])
             if test "x$SDL_CONFIG" = "xnone"; then
                 AC_MSG_ERROR([*** sdl-config not found!
Either SDL is not installed, or is installed in a different path.
You may correct the problem by either:
  - installing SDL if it is not installed. Get SDL from www.libsdl.org.
  - setting the SDL_CONFIG variable to the path of the sdl-config program
  - setting the SDL_LIBS variable to the compilation flags required to
    link to the SDL libraries, and SDL_CFLAGS to the compilation flags
    required to compile with the SDL headers.
    (recommanded on platforms where sdl-config is
    not available, like the native Mac OS X SDL version)])
             else
	       # use sdl-config to build SDL_LIBS
               if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL.la
                 then SDL_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL.la
                 else SDL_LIBS=`$SDL_CONFIG --libs`
               fi
               SDL_CFLAGS=`$SDL_CONFIG --cflags`
               SDL_VERSION=`$SDL_CONFIG --version`
             fi
    esac
    fi
fi

# Freetype is only required for the OpenGL backend
if test "x$ENABLE_OPENGL_GFX" = "xyes"; then
if test "x$FREETYPE_LIBS" = "x"; then
   # Check for FREETYPE_CONFIG
   if test "x$FREETYPE_CONFIG" = "x"; then
   case $host_os in
     	 *)
	     AC_PATH_PROGS([FREETYPE_CONFIG], [freetype-config], [none])
             if test "x$FREETYPE_CONFIG" = "xnone"; then
                 AC_MSG_ERROR([*** freetype-config not found!
Either freetype-devel is not installed, or is installed in a different path.
You may correct the problem by either:
  - installing freetype-devel if it is not installed.
  - setting the FREETYPE_CONFIG variable to the path of the freetype-config program
  - setting the FREETYPE_LIBS variable to the compilation flags required to
    link to the freetype libraries, and FREETYPE_CFLAGS to the compilation flags
    required to compile with the freetype headers.])
             else
	       # use freetype-config to build FREETYPE_LIBS
               FREETYPE_LIBS=`$FREETYPE_CONFIG --libs`
               FREETYPE_CFLAGS=`$FREETYPE_CONFIG --cflags`
             fi
    esac
    fi
fi
fi

#######################################################################
# Checks for libraries.                                               #
#######################################################################
#if test "x$SDL_CONFIG" = "xnone"; then
#    case $host_os in
#        darwin*)
#	    SDL_LIBS="-framework SDL -framework SDL_image -framework SDL_mixer -framework SDL_ttf -framework Cocoa"
#            ENABLE_SDL12_GFX="true"
#            FRAMEWORKS_DIR="$SYSTEM_ROOT/Library/Frameworks"
#	    SDL_CFLAGS="$SDL_LIBS"
#"-I$FRAMEWORKS_DIR/SDL.framework/Headers -I$FRAMEWORKS_DIR/SDL_image.framework/Headers -I$FRAMEWORKS_DIR/SDL_mixer.framework/Headers -I$FRAMEWORKS_DIR/SDL_ttf.framework/Headers"
#    esac
#if test "x$SDL_CONFIG" = "x"; then
#    if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL.la
#    then SDL_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL.la
#    else SDL_LIBS=`$SDL_CONFIG --libs`
#    fi
#    SDL_CFLAGS=`$SDL_CONFIG --cflags`
#fi

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"

OLD_LIBS=$LIBS
#LIBS="$SDL_LIBS"
LDFLAGS="$LDFLAGS $SDL_LIBS"

case "$SDL_VERSION" in
    1.2.*)
        ENABLE_SDL12_GFX="true"
        ;;
    1.3.*)
        ENABLE_SDL13_GFX="true"
        ;;
esac
AC_SEARCH_LIBS([SDL_Init],
               [SDL],,
               [AC_MSG_ERROR([*** SDL lib not found! Get SDL from
http://www.libsdl.org])])

ac_link="$LDPREFIX $ac_link"
AC_SEARCH_LIBS([IMG_Load],
               [SDL_image],,
	       [AC_MSG_ERROR([*** SDL_image lib not found! Get SDL_image from
http://www.libsdl.org/projects/SDL_image/index.html])])

AC_SEARCH_LIBS([Mix_OpenAudio],
               [SDL_mixer],,
	     [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])

AC_SEARCH_LIBS([TTF_Init],
               [SDL_ttf],,
	     [AC_MSG_ERROR([*** SDL_ttf lib not found! Get SDL_ttf from
http://www.libsdl.org/projects/SDL_ttf/index.html])])

AC_SEARCH_LIBS([glGetError], [GL])

AC_SEARCH_LIBS([png_set_read_fn], [png])

AC_SEARCH_LIBS([jpeg_read_header], [jpeg])

AC_SEARCH_LIBS([FT_Load_Glyph], [freetype])

AC_SEARCH_LIBS([pthread_mutex_trylock],
               [pthread],,
	     [AC_MSG_ERROR([*** pthread lib not found!])])

SDL_LIBS=$LIBS
LIBS=$OLD_LIBS

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_IMAGE_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_TTF_LIBS])
AC_SUBST([GL_LIBS])

AC_SEARCH_LIBS([gethostbyname], [nsl])

#######################################################################
# Checks for header files.                                            #
#######################################################################

AC_HEADER_DIRENT
AC_HEADER_STDC

AC_LANG([C++])
# Headers for google sparse hash
# Figure out where hash_map lives and also hash_fun.h (or stl_hash_fun.h).
# This also tells us what namespace hash code lives in.
AC_CXX_STL_HASH
AC_CXX_STL_HASH_FUN

# SDL headers
AC_CHECK_HEADERS([SDL/SDL.h],
                 [SDL_H_FOUND=1; break],
                 [])
if test "x$SDL_H_FOUND" = "x"; then
    AC_MSG_ERROR([*** SDL include files not found!
You should install SDL development package.])
fi

# SDL_image headers
AC_CHECK_HEADERS([SDL/SDL_image.h SDL_image/SDL_image.h],
                 [SDL_IMAGE_H_FOUND=1; break],
                 [])
if test "x$SDL_IMAGE_H_FOUND" = "x"; then
    AC_MSG_ERROR([*** SDL_image include files not found!
You should install SDL_image development package.])
fi

# SDL_mixer headers
AC_CHECK_HEADERS([SDL/SDL_mixer.h SDL_mixer/SDL_mixer.h],
                 [SDL_MIXER_H_FOUND=1; break],
                 [])
if test "x$SDL_MIXER_H_FOUND" = "x"; then
    AC_MSG_ERROR([*** SDL_mixer include files not found!
You should install SDL_mixer development package.])
fi

# SDL_ttf
AC_CHECK_HEADERS([SDL/SDL_ttf.h SDL_ttf/SDL_ttf.h],
                 [SDL_TTF_H_FOUND=1; break],
                 [])
if test "x$SDL_TTF_H_FOUND" = "x"; then
    AC_MSG_ERROR([*** SDL_ttf include files not found!
You should install SDL_ttf development package.])
fi

# OpenGL support if enabled
if test "x$ENABLE_OPENGL_GFX" = "xyes"; then
AC_CHECK_HEADERS([GL/gl.h OpenGL/gl.h],
                 [GL_H_FOUND=1; break],
                 [])
if test "x$GL_H_FOUND" = "x"; then
    ENABLE_OPENGL_GFX=no
fi
# OpenGL support requires freetype2 headers, TODO: add proper check
CXXFLAGS="$CXXFLAGS $FREETYPE_CFLAGS"
AC_CHECK_HEADERS([GL/glext.h OpenGL/glext.h],
                 [GLEXT_H_FOUND=1; break], [],
                 [#ifdef HAVE_GL_GL_H
#include <GL/gl.h>
#endif
#ifdef HAVE_OPENGL_GL_H
#include <OpenGL/gl.h>
#endif
])
if test "x$GLEXT_H_FOUND" = "x"; then
    ENABLE_OPENGL_GFX=no
fi

fi

AC_CHECK_HEADER(sqlite3.h, ENABLE_SQLITE=true, ENABLE_SQLITE=false)

CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS

AC_SUBST([SDL_CFLAGS])

AC_CHECK_HEADERS([stdlib.h unistd.h poll.h sys/poll.h sys/select.h])

#######################################################################
# Checks for typedefs, structures, and compiler characteristics.      #
#######################################################################

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([floor socket strtoul])

AC_CHECK_MEMBERS([struct sockaddr.sa_len], , ,
[
#include <sys/types.h>
#include <sys/socket.h>
])

# Checks for header files.

#######################################################################
# Automake conditionals                                               #
#######################################################################

# Features summary
if test "x$ENABLE_AUDIO" = "xtrue"; then
  AC_MSG_RESULT([Audio Enabled])
  FPFEATURES="-DUSE_AUDIO=1"
else
  AC_MSG_RESULT([Audio Disabled])
fi

if test "x$ENABLE_TTF" = "xtrue"; then
  AC_MSG_RESULT([TTF Enabled])
  FPFEATURES="$FPFEATURES -DENABLE_TTF=1"
else
  AC_MSG_RESULT([TTF Disabled])
fi

if test "x$ENABLE_NULL_BACKENDS" = "xyes"; then
  AC_MSG_RESULT([Null Backends Enabled])
  FPFEATURES="$FPFEATURES -DNULL_BACKENDS=1"
  ENABLE_OPENGL_GFX=no
  ENABLE_SDL12_GFX=no
  ENABLE_SDL13_GFX=no
fi

if test "x$ENABLE_OPENGL_GFX" = "xyes"; then
  AC_MSG_RESULT([OpenGL Enabled])
  FPFEATURES="$FPFEATURES -DOPENGL_GFX=1"
else
  AC_MSG_RESULT([OpenGL Disabled])
fi

if test "x$ENABLE_SDL12_GFX" = "xtrue"; then
  AC_MSG_RESULT([SDL 1.2 Enabled])
  FPFEATURES="$FPFEATURES -DSDL12_GFX=1"
else
  AC_MSG_RESULT([SDL 1.2 Disabled])
fi

if test "x$ENABLE_SDL13_GFX" = "xtrue"; then
  AC_MSG_RESULT([SDL 1.3 Enabled])
  FPFEATURES="$FPFEATURES -DSDL13_GFX=1"
else
  AC_MSG_RESULT([SDL 1.3 Disabled])
fi

if test "x$ENABLE_NETWORK" = "xyes"; then
  AC_MSG_RESULT([Network Gaming Enabled])
  FPFEATURES="$FPFEATURES -DENABLE_NETWORK=1"
else
  AC_MSG_RESULT([Network Gaming Disabled])
fi

if test "x$ENABLE_NETWORK_INTERNET" = "xyes"; then
  AC_MSG_RESULT([Network Gaming (Internet) Enabled])
  FPFEATURES="$FPFEATURES -DENABLE_NETWORK_INTERNET=1"
else
  AC_MSG_RESULT([Network Gaming (Internet) Disabled])
fi

if test "x$ENABLE_SQLITE" = "xtrue"; then
  AC_MSG_RESULT([Network Gaming Server (Internet) Enabled])
  FPFEATURES="$FPFEATURES -DENABLE_NETWORK_SERVER_INTERNET=1"
else
  AC_MSG_RESULT([Network Gaming Server (Internet) Disabled])
fi

case $host_os in
   darwin*)
       MACOSX=true
       ;;
   mingw*)
       WIN32=true
       LIBS="$LIBS -lws2_32"
esac

AM_CONDITIONAL([MACOSX], [test x$MACOSX = xtrue])
AM_CONDITIONAL([WIN32], [test x$WIN32 = xtrue])

AM_CONDITIONAL([ENABLE_OPENGL_GFX], [test x$ENABLE_OPENGL_GFX = xyes])
AM_CONDITIONAL([ENABLE_SDL12_GFX], [test x$ENABLE_SDL12_GFX = xtrue])
AM_CONDITIONAL([ENABLE_SDL13_GFX], [test x$ENABLE_SDL13_GFX = xtrue])
AM_CONDITIONAL([ENABLE_NETWORK], [test x$ENABLE_NETWORK = xyes])
AM_CONDITIONAL([ENABLE_NETWORK_INTERNET], [test x$ENABLE_NETWORK_INTERNET = xyes])
AM_CONDITIONAL([ENABLE_NETWORK_SERVER], [test x$ENABLE_SQLITE = xtrue])
AM_CONDITIONAL([ENABLE_MAC_RECEIPT], [test x$ENABLE_MAC_RECEIPT = xyes])

AM_CONDITIONAL([ENABLE_EMSCRIPTEN], [test x$ENABLE_EMSCRIPTEN = xyes])

AC_SUBST([SYSTEM_ROOT])
AC_SUBST([FPFEATURES])

AC_CONFIG_FILES([
Makefile
common/Makefile
fpcore/Makefile
gametools/Makefile
gametools/sdl_drawcontext/Makefile
gametools/opengl_drawcontext/Makefile
iosfc/Makefile goomsl/Makefile
data/Makefile
data/base.000/Makefile
data/base.000/gfx/Makefile
data/base.000/gfx/balloon/Makefile
data/base.000/gfx/city/Makefile
data/base.000/gfx/etherdown/Makefile
data/base.000/gfx/kaori_gamelost/Makefile
data/base.000/gfx/kaori_title/Makefile
data/base.000/gfx/netfailure/Makefile
data/base.000/gfx/progressbar/Makefile
data/base.000/gfx/sdkaori/Makefile
data/base.000/gfx/title/Makefile
data/base.000/gfx/unmappedmonster/Makefile
data/base.000/lib/Makefile
data/base.000/locale/Makefile
data/base.000/locale/ar/Makefile
data/base.000/locale/en/Makefile
data/base.000/locale/fr/Makefile
data/base.000/music/Makefile
data/base.000/sfx/Makefile
data/base.000/story/Makefile
data/base.000/story/fx/Makefile
data/base.000/theme/Makefile
data/base.000/theme/Classic.fptheme/Makefile
data/base.000/theme/Classic.fptheme/locale/Makefile
data/base.000/theme/Classic.fptheme/locale/en/Makefile
data/base.000/theme/Classic.fptheme/locale/fr/Makefile
data/reduced.001/Makefile
data/reduced.001/gfx/Makefile
data/reduced.001/gfx/ending/Makefile
data/reduced.001/gfx/introlevel/Makefile
data/reduced.001/story/Makefile
data/reduced.001/theme/Makefile
data/reduced.001/music/Makefile
data/reduced.001/theme/Reduced.fptheme/Makefile
data/reduced.001/theme/Reduced.fptheme/locale/Makefile
data/reduced.001/theme/Reduced.fptheme/locale/en/Makefile
data/reduced.001/theme/Reduced.fptheme/locale/fr/Makefile
fpserver/Makefile
mac/Makefile
])
AC_OUTPUT
