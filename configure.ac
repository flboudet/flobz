#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#######################################################################
# Configuration                                                       #
#######################################################################

AC_INIT([flobopuyo], [1.0], [flobo@ios-software.com])
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([main.cpp])
AC_CONFIG_HEADERS([config.h])

ENABLE_AUDIO=true
#DISABLE_NETWORK?=false
#ENABLE_OPENGL?=false
ENABLE_TTF=true
#ENABLE_BENCHMARKS?=false

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
AC_PROG_LEX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Check for SDL_CONFIG
case $host_os in
     darwin*)
         SDL_CONFIG=none
         ;;
     *)
         AC_PATH_PROGS([SDL_CONFIG], [sdl-config sdl11-config], [none])
         if test "x$SDL_CONFIG" = "xnone"; then
             AC_MSG_ERROR([*** SDL not found! Get SDL from www.libsdl.org.
If you already installed it, check it's in the path. If problem remains,
please send a mail to the address that appears in ./configure --version
indicating your platform, the version of configure script and the problem.])
         fi
esac

#######################################################################
# Checks for libraries.                                               #
#######################################################################
if test "x$SDL_CONFIG" = "xnone"; then
    case $host_os in
        darwin*)
	    SDL_LIBS="-framework SDL -framework SDL_image -framework SDL_mixer -framework SDL_ttf -framework Cocoa"
    esac
else
    if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL.la
    then SDL_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL.la
    else SDL_LIBS=`$SDL_CONFIG --libs`
    fi
fi
OLD_LIBS=$LIBS
LIBS="$LIBS $SDL_LIBS"

AC_SEARCH_LIBS([SDL_Init], [SDL])

ac_link="$LDPREFIX $ac_link"
AC_SEARCH_LIBS([IMG_Load],
               [SDL_image], [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_image.la
then SDL_IMAGE_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_image.la
else SDL_IMAGE_LIBS=-lSDL_image
fi],
	       [AC_MSG_ERROR([*** SDL_image lib not found! Get SDL_image from
http://www.libsdl.org/projects/SDL_image/index.html])])

AC_SEARCH_LIBS([Mix_OpenAudio],
               [SDL_mixer],
	          [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
then SDL_MIXER_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_mixer.la
else SDL_MIXER_LIBS=-lSDL_mixer
fi],
	     [AC_MSG_ERROR([*** SDL_mixer lib not found! Get SDL_mixer from
http://www.libsdl.org/projects/SDL_mixer/index.html])])

AC_SEARCH_LIBS([TTF_Init],
               [SDL_ttf],
	          [if test -n "$LDPREFIX" -a -r `$SDL_CONFIG --prefix`/lib/libSDL_ttf.la
then SDL_TTF_LIBS=`$SDL_CONFIG --prefix`/lib/libSDL_ttf.la
else SDL_TTF_LIBS=-lSDL_ttf
fi],
	     [AC_MSG_ERROR([*** SDL_ttf lib not found! Get SDL_ttf from
http://www.libsdl.org/projects/SDL_ttf/index.html])])

LIBS=$OLD_LIBS

AC_SUBST([SDL_LIBS])
AC_SUBST([SDL_IMAGE_LIBS])
AC_SUBST([SDL_MIXER_LIBS])
AC_SUBST([SDL_TTF_LIBS])

AC_SEARCH_LIBS([gethostbyname], [nsl])

#######################################################################
# Checks for header files.                                            #
#######################################################################

AC_HEADER_DIRENT
AC_HEADER_STDC

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS

if test "x$SDL_CONFIG" = "xnone"; then
    case $host_os in
        darwin*)
            SYSTEM_ROOT="/Developer/SDKs/MacOSX10.4u.sdk"
            FRAMEWORKS_DIR="$SYSTEM_ROOT/Library/Frameworks"
	    SDL_CFLAGS="-I$FRAMEWORKS_DIR/SDL.framework/Headers -I$FRAMEWORKS_DIR/SDL_image.framework/Headers -I$FRAMEWORKS_DIR/SDL_mixer.framework/Headers -I$FRAMEWORKS_DIR/SDL_ttf.framework/Headers"
    esac
else
  SDL_CFLAGS=`$SDL_CONFIG --cflags`
  SDL_CFLAGS="$SDL_CFLAGS"
fi
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"

AC_CHECK_HEADER([SDL.h],
		[],
                [AC_MSG_ERROR([*** SDL include files not found!
You should install SDL development package.])])

AC_CHECK_HEADER([SDL_image.h],
		[],
			[AC_MSG_ERROR([*** SDL_image include files not found!
You should install development package.])])

AC_CHECK_HEADER([SDL_mixer.h],
		[],
			[AC_MSG_ERROR([*** SDL_mixer include files not found!
You should install development package.])])

AC_CHECK_HEADER([SDL_ttf.h],
		[],
			[AC_MSG_ERROR([*** SDL_ttf include files not found!
You should install development package.])])

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
  ['#include <sys/types.h>
    #include <sys/socket.h>'])

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

case $host_os in
   darwin*)
       MACOSX=true
esac
AM_CONDITIONAL([MACOSX], [test x$MACOSX = xtrue])

AC_SUBST([FPFEATURES])

AC_CONFIG_FILES([
  Makefile gametools/Makefile iosfc/Makefile goomsl/Makefile
])
AC_OUTPUT
