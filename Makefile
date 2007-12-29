# Makefile
# author: iOS-Software
# July 2004

###############
# Settings

OBJFILES= audio.o HiScores.o PuyoCommander.o \
          main.o PuyoDataPathManager.o PuyoGame.o PuyoDoomMelt.o \
          PuyoView.o PuyoAnimations.o AnimatedPuyo.o PuyoIA.o \
          styrolyse.o PuyoStory.o PuyoChatBox.o PuyoLocalizedDictionary.o \
          PuyoStarter.o PuyoSinglePlayerStarter.o PuyoTwoPlayerStarter.o \
          PuyoNetworkStarter.o PuyoNetworkView.o PuyoNetworkGame.o \
          AnimatedPuyoTheme.o PuyoNetworkMenu.o PuyoNetCenterMenu.o \
          PuyoNetGameCenter.o PuyoInternetGameCenter.o PuyoLanGameCenter.o\
          PuyoLocalMenu.o PuyoOptionMenu.o PuyoControlMenu.o \
          LevelThemeMenu.o PuyoThemeMenu.o PuyoScreenTransition.o \
          PuyoNatTraversal.o

include root_dir
include config

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = $(CXX) -MM $(DEPFLAGS) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CPPFLAGS) -f $*.d $<

all: flobopuyo
flobopuyo: all.h.gch iosfc_dir gametools_dir goomsl_dir ${OBJFILES}
	@echo "Compiled with CFLAGS=${CFLAGS}"
	@echo "[flobopuyo]" && $(CXX) $(CFLAGS) -o $(PRGNAME) ${OBJFILES} gametools/*.o goomsl/goomsl*.o -Liosfc -liosfc $(LDFLAGS)
	@echo "--------------------------------------"
	@echo " Compilation finished"
	@echo
	@echo " Type ./$(PRGNAME) to play."
	@echo "--------------------------------------"

iosfc_dir:all.h.gch
	@+DEPFLAGS='$(DEPFLAGS)' CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) CXX=$(CXX) CFLAGS_NOPCH='$(CFLAGS_NOPCH)' make -C iosfc libiosfc.a

gametools_dir:all.h.gch
	@+DEPFLAGS='$(DEPFLAGS)' CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) CXX=$(CXX) CFLAGS_NOPCH='$(CFLAGS_NOPCH)' make -C gametools object

goomsl_dir:all.h.gch
	@+DEPFLAGS='$(DEPFLAGS)' CFLAGS='$(CFLAGS)' CXXFLAGS='$(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' CC=$(CC) AR=$(AR) RANLIB=$(RANLIB) CXX=$(CXX) CFLAGS_NOPCH='$(CFLAGS_NOPCH)' make -C goomsl object

%.o:%.cpp all.h.gch
	@mkdir -p $(DEPDIR);\
	$(MAKEDEPEND); \
	cp $(df).d $(df).P; \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)

all.h.gch:
	@echo "Compiling with CFLAGS=$(CFLAGS)"
	@echo "Compiling with CFLAGS_NOPCH=$(CFLAGS_NOPCH)"
	@echo "Compiling with LDFLAGS=$(LDFLAGS)"
	@echo "Compiling with DEPFLAGS=$(DEPFLAGS)"
	@echo "[Precompiling Headers]"
	@mkdir -p $(DEPDIR);\
	$(CXX) -MM $(CFLAGS_NOPCH) -o $(DEPDIR)/all.d all.h ;\
	cat $(DEPDIR)/all.d | sed 's/all.o/all.h.gch/' > $(DEPDIR)/all.P; \
	cat $(DEPDIR)/all.d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(DEPDIR)/all.P; \
	rm -f $(DEPDIR)/all.d
	@echo "[$@]" && $(CXX) $(CFLAGS_NOPCH) all.h # 2>> WARNINGS || (cat WARNINGS && false)

#glSDL.o:glSDL.cpp all.h.gch 
#	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< 2>> EXT_WARNINGS
#	@rm -f EXT_WARNINGS
corona.o:corona.cpp all.h.gch 
corona32.o:corona32.cpp all.h.gch 
corona_palette.o:corona_palette.cpp	 all.h.gch 

clean:
	rm -rf *~ *.o flobopuyo* $(PRGNAME) WARNINGS *.gch
	rm -rf .xvpics data/.xvpics data/*/.xvpics data/*/*/.xvpics
	rm -rf $(bundle_name)
	rm -rf $(macimage_name) $(macimage_name).dmg $(macimage_name).tmp.dmg $(WINZIP_NAME) $(WINZIP_NAME).zip
	rm -f  .DS_Store */.DS_Store */*/.DS_Store */*/*/.DS_Store
	rm -f  .gdb_history
	+make -C iosfc clean
	+make -C gametools clean
	+make -C goomsl clean

install: flobopuyo
	$(STRIP) $(PRGNAME)
	mkdir -p ${INSTALL_BINDIR}
	mkdir -p ${INSTALL_DATADIR}
	@+make -C data install INSTALL_DATADIR="${INSTALL_DATADIR}" ADDITIONAL_DIRECTORIES="$(data-packages)"
	chmod a+rx ${INSTALL_DATADIR}
	chmod a+rx ${INSTALL_DATADIR}/sfx
	chmod a+rx ${INSTALL_DATADIR}/gfx
	chmod a+rx ${INSTALL_DATADIR}/story
	chmod -R a+r ${INSTALL_DATADIR}
	cp ./$(PRGNAME) ${INSTALL_BINDIR}/flobopuyo
	chmod a+rx ${INSTALL_BINDIR}/flobopuyo

bundle: flobopuyo
	rm -Rf $(bundle_name) 
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources
	mkdir -p $(bundle_name)/Contents/Frameworks
	cp -RP $(FRAMEWORKS_DIR)/SDL* $(bundle_name)/Contents/Frameworks/
	$(STRIP) -x $(bundle_name)/Contents/Frameworks/SDL*/SDL*
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	sed "s/@@VERSION@@/$(VERSION)/" mac/Info.plist > $(bundle_name)/Contents/Info.plist
	cp mac/icon.icns $(bundle_name)/Contents/Resources/
	cp flobopuyo $(bundle_name)/Contents/MacOS/flobopuyo
	@+make -C data install INSTALL_DATADIR="$(PWD)/$(bundle_name)/Contents/Resources/data" ADDITIONAL_DIRECTORIES="$(data-packages)"
	$(STRIP) -u -r $(bundle_name)/Contents/MacOS/flobopuyo

mac-package: bundle
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

win-package: flobopuyo
	$(STRIP) $(PRGNAME)
	mkdir -p $(WINZIP_NAME)
	@+make -C data install INSTALL_DATADIR="$(PWD)/$(WINZIP_NAME)/data" ADDITIONAL_DIRECTORIES="$(data-packages)"
	cp flobopuyo.exe $(WINZIP_NAME)
	cp COPYING $(WINZIP_NAME)
	cp $(WINSDLDEVLIBS)/*.dll $(WINZIP_NAME)
	zip -9 -r $(WINZIP_NAME) $(WINZIP_NAME)
	rm -rf $(WINZIP_NAME)

.PHONY: all clean iosfc_dir gametools_dir goomsl_dir

-include $(OBJFILES:%.o=$(DEPDIR)/%.P) $(DEPDIR)/all.P

ctags:
	ctags --languages=c++ -R .
