ACLOCAL_AMFLAGS=-I m4

bin_PROGRAMS=flobopop
flobopop_SOURCES=
flobopop_CXXFLAGS = -I$(top_srcdir)/iosfc -I$(top_srcdir)/gametools -I$(top_srcdir)/goomsl $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)
flobopop_OBJCFLAGS= $(SDL_CFLAGS)
flobopop_LDADD=fpcore/libfpcore.la gametools/libgametools.la goomsl/libgoomsl.la iosfc/libiosfc.la

# Force the C++ linker
nodist_EXTRA_flobopop_SOURCES=dummy.cpp

EXTRA_DIST=

# Graphic backend configuration
if ENABLE_SDL12_GFX
flobopop_LDADD+=gametools/sdl_drawcontext/libsdl12_drawcontext.la
endif

if ENABLE_SDL13_GFX
flobopop_LDADD+=gametools/sdl_drawcontext/libsdl13_drawcontext.la
endif

if ENABLE_OPENGL_GFX
flobopop_LDADD+=gametools/opengl_drawcontext/libopengl_drawcontext.la
endif

flobopop_LDADD+=$(SDL_LIBS) -lpng

SUBDIRS = iosfc gametools goomsl common fpcore data fpserver mobile

if MACOSX
SUBDIRS+=mac
flobopop_LDADD+=mac/libfpmac.la
endif

# Mac OS X Targets

bundle_name = $(abs_top_builddir)/FloboPop.app
macimage_name=flobopop-$(VERSION)-mac$(REVISION)

mac-bundle: flobopop
	rm -Rf "$(bundle_name)"
	mkdir -p "$(bundle_name)/Contents/MacOS"
	mkdir -p "$(bundle_name)/Contents/Resources"
	mkdir -p "$(bundle_name)/Contents/Frameworks"
	echo "APPL????" > "$(bundle_name)/Contents/PkgInfo"
	sed "s/@[@]VERSION[@]@/$(VERSION)$(REVISION)/" $(top_srcdir)/mac/Info.plist > "$(bundle_name)/Contents/Info.plist"
	cp $(abs_top_srcdir)/mac/icon.icns "$(bundle_name)/Contents/Resources/"
	cp $(bin_PROGRAMS) "$(bundle_name)/Contents/MacOS/"
	$(MAKE) $(AM_MAKEFLAGS) install fpdata="$(bundle_name)/Contents/Resources/data" datarootdir="$(bundle_name)/Contents/Resources" bindir="$(bundle_name)/Contents/MacOS"
	for FWNAME in $(ADDITIONAL_FRAMEWORKS) ; do \
	  cp -RP "$(ADDITIONAL_FRAMEWORKS_DIRECTORY)/$${FWNAME}.framework" "$(bundle_name)/Contents/Frameworks"; \
	done

mac-package: mac-bundle
	rm -rf $(macimage_name)
	rm -f $(macimage_name).dmg
	rm -f $(macimage_name).tmp.dmg
	mkdir -p $(macimage_name)
	cp -RP $(bundle_name) $(macimage_name)
	cp COPYING $(macimage_name)
	hdiutil create -srcfolder $(macimage_name) $(macimage_name).tmp.dmg
	hdiutil convert -format UDZO -imagekey zlib-level=9 -o $(macimage_name).dmg $(macimage_name).tmp.dmg
	hdiutil internet-enable $(macimage_name).dmg
	rm -rf $(macimage_name)
	rm -f $(macimage_name).tmp.dmg

# Windows targets

windistdir = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32$(REVISION)"
wininstaller = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)$(REVISION)-setup.exe"
winziparchive = "$(abs_top_builddir)/$(PACKAGE)-$(VERSION)-win32$(REVISION).zip"
win_dll_to_copy=jpeg.dll libpng12-0.dll SDL.dll SDL_mixer.dll zlib1.dll \
libfreetype-6.dll libtiff-3.dll SDL_image.dll SDL_ttf.dll

win_binary_distribution:
	rm -rf $(windistdir)
	mkdir $(windistdir)
	$(MAKE) $(AM_MAKEFLAGS) install fpdata="$(windistdir)/data" datarootdir="$(windistdir)" bindir="$(windistdir)"
	cp COPYING $(windistdir)
	for curlib in $(win_dll_to_copy); do find `dirname \`dirname "$(SDL_CONFIG)"\`` -name "$${curlib}" -exec cp {} $(windistdir) \; ; done

win_installer: win_binary_distribution
	cd $(windistdir) \
	&& makensis -NOCD $(abs_top_srcdir)/win32/installer.nsi \
	&& mv FloboPopSetup.exe $(wininstaller)

win_zip_archive: win_binary_distribution
	rm -f $(winziparchive)
	cd `dirname $(windistdir)` \
	&& zip -9 -r $(winziparchive) `basename $(windistdir)`

