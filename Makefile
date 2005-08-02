# Makefile
# author: iOS-Software
# July 2004

###############
# Settings

OBJFILES= HiScores.o PuyoCommander.o        \
          main.o PuyoGame.o PuyoVersion.o         \
          PuyoView.o PuyoAnimations.o AnimatedPuyo.o PuyoIA.o    \
          audio.o styrolyse.o PuyoStory.o \
          PuyoDoomMelt.o corona32.o corona.o corona_palette.o\
          PuyoStarter.o PuyoSinglePlayerStarter.o PuyoTwoPlayerStarter.o     \
          PuyoNetworkStarter.o PuyoNetworkView.o PuyoNetworkGame.o \
          AnimatedPuyoTheme.o PuyoNetworkMenu.o PuyoNetCenterMenu.o \
          PuyoNetGameCenter.o PuyoInternetGameCenter.o \
          PuyoLocalMenu.o PuyoOptionMenu.o PuyoControlMenu.o PuyoThemeMenu.o PuyoScreenTransition.o IosVector.o

include root_dir
include config

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = ${CXX} -MM $(CFLAGS_NOPCH) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CPPFLAGS) -f $*.d $<

all: prelude flobopuyo

flobopuyo: prelude iosfc_dir gametools_dir goomsl_dir ${OBJFILES}
	@echo "[flobopuyo]" && $(CXX) $(CFLAGS) $(LDFLAGS) -o $(PRGNAME) -lSDL_mixer -lSDL_image ${OBJFILES} iosfc/*.o gametools/*.o goomsl/goomsl*.o
	@echo "--------------------------------------"
	@echo " Compilation finished"
	@echo
	@echo " Type ./$(PRGNAME) to play."
	@echo "--------------------------------------"

prelude:all.h.gch
	@echo "Compiling with CFLAGS=$(CFLAGS)"
	@echo "Compiling with LDFLAGS=$(LDFLAGS)"

iosfc_dir:all.h.gch
	@+CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)' CXX=$(CXX) CFLAGS_NOPCH='$(CFLAGS_NOPCH)' make -C iosfc object

gametools_dir:all.h.gch
	@+CFLAGS='$(CFLAGS)' LDFLAGS='$(LDFLAGS)' CXX=$(CXX) CFLAGS_NOPCH='$(CFLAGS_NOPCH)' make -C gametools object

goomsl_dir:all.h.gch
	@+make -C goomsl object

%.o:%.cpp all.h.gch
	@mkdir -p $(DEPDIR);\
	$(MAKEDEPEND); \
	cp $(df).d $(df).P; \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

all.h.gch:
	@echo "[Precompiling Headers]"
	@mkdir -p $(DEPDIR);\
	${CXX} -MM $(CFLAGS_NOPCH) -o $(DEPDIR)/all.d all.h ;\
	cat $(DEPDIR)/all.d | sed 's/all.o/all.h.gch/' > $(DEPDIR)/all.P; \
	cat $(DEPDIR)/all.d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(DEPDIR)/all.P; \
	rm -f $(DEPDIR)/all.d
	@echo "[$@]" && $(CXX) $(CFLAGS_NOPCH) all.h # 2>> WARNINGS || (cat WARNINGS && false)

glSDL.o:glSDL.cpp all.h.gch 
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< 2>> EXT_WARNINGS
	@rm -f EXT_WARNINGS
corona.o:corona.cpp all.h.gch 
corona32.o:corona32.cpp all.h.gch 
corona_palette.o:corona_palette.cpp	 all.h.gch 

clean:
	rm -rf .DS_Store */.DS_Store */*/.DS_Store */*/*/.DS_Store
	rm -rf *~ *.o flobopuyo* $(PRGNAME) WARNINGS *.gch
	rm -rf .xvpics data/.xvpics    data/*/.xvpics
	rm -rf $(bundle_name)
	rm -rf $(macimage_name)
	rm -f  $(macimage_name).dmg
	rm -f  .DS_Store */.DS_Store */*/.DS_Store .gdb_history
	+make -C iosfc clean
	+make -C gametools clean
	+make -C goomsl clean

install: flobopuyo
	$(STRIP) $(PRGNAME)
	mkdir -p ${INSTALL_BINDIR}
	mkdir -p ${INSTALL_DATADIR}
	cp -r data/* ${INSTALL_DATADIR}
	chmod a+rx ${INSTALL_DATADIR}
	chmod a+rx ${INSTALL_DATADIR}/sfx
	chmod a+rx ${INSTALL_DATADIR}/gfx
	chmod a+rx ${INSTALL_DATADIR}/story
	chmod -R a+r ${INSTALL_DATADIR}
	cp ./$(PRGNAME) ${INSTALL_BINDIR}/flobopuyo
	chmod a+rx ${INSTALL_BINDIR}/flobopuyo

flobopuyo-static: prelude iosfc_dir gametools_dir goomsl_dir ${OBJFILES}
	@echo "[flobopuyo-static]" && $(CXX) $(CFLAGS) -o flobopuyo-static ${OBJFILES} iosfc/*.o gametools/*.o goomsl/goomsl*.o /sw/lib/libSDL_mixer.a /sw/lib/libSDL_net.a /sw/lib/libvorbisfile.a /sw/lib/libvorbis.a /sw/lib/libogg.a /sw/lib/libsmpeg.a /sw/lib/libSDL_image.a /sw/lib/libjpeg.a /sw/lib/libpng.a -lz -framework CoreFoundation `$(SDL_CONFIG) --static-libs`
	@echo "--------------------------------------"
	@[ -s WARNINGS ] && echo -e "--------------------------------------\n There have been some warnings:\n" && cat WARNINGS && rm -f WARNINGS && echo "--------------------------------------" || true
	@echo "--------------------------------------"
	@echo " Compilation finished"

bundle: flobopuyo-static
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	sed "s/@@VERSION@@/$(VERSION)/" mac/Info.plist > $(bundle_name)/Contents/Info.plist
	cp mac/icon.icns $(bundle_name)/Contents/Resources/
	cp flobopuyo-static $(bundle_name)/Contents/MacOS/flobopuyo
	cp -r data $(bundle_name)/Contents/Resources
	rm -rf $(bundle_name)/Contents/Resources/data/CVS $(bundle_name)/Contents/Resources/data/*/CVS
	rm -rf $(bundle_name)/Contents/Resources/data/.xvpics $(bundle_name)/Contents/Resources/data/*/.xvpics
	rm -f $(bundle_name)/Contents/Resources/data/.DS_Store $(bundle_name)/Contents/Resources/data/*/.DS_Store
	$(STRIP) $(bundle_name)/Contents/MacOS/flobopuyo

mac-package: bundle
	mkdir -p $(macimage_name)
	cp -r $(bundle_name) $(macimage_name)
	cp COPYING $(macimage_name)
	hdiutil create -srcfolder $(macimage_name) $(macimage_name).dmg
	hdiutil internet-enable $(macimage_name).dmg

win-package: flobopuyo
	$(STRIP) $(PRGNAME)
	mkdir -p $(WINZIP_NAME)
	cp -r data $(WINZIP_NAME)
	cp flobopuyo.exe $(WINZIP_NAME)
	cp COPYING $(WINZIP_NAME)
	cp $(WINSDLRUNTIME)/*.dll $(WINZIP_NAME)
	zip -r $(WINZIP_NAME) $(WINZIP_NAME)

.PHONY: all clean

-include $(OBJFILES:%.o=$(DEPDIR)/%.P) $(DEPDIR)/all.P

ctags:
	ctags --languages=c++ -R .
