OBJFILES=SDL_Painter.o InputManager.o GameControls.o GameCursor.o \
         IosImgProcess.o preferences.o sofont.o gameloop.o gameui.o \
         ListView.o Frame.o FramedButton.o mt19937ar.o

object:${OBJFILES}

DEPDIR = .deps
df = $(DEPDIR)/$(*F)

MAKEDEPEND = ${CXX} -MM $(CFLAGS_NOPCH) -o $(df).d $<
# MAKEDEPEND = touch $*.d && makedepend $(CXXFLAGS) -f $*.d $<

%.o:%.cpp
	@mkdir -p $(DEPDIR); \
	$(MAKEDEPEND);  \
	cp $(df).d $(df).P;  \
	cat $(df).d | sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	-e '/^$$/ d' -e 's/$$/ :/' >> $(df).P; \
	rm -f $(df).d
	@echo "[$@]" && $(CXX) $(CFLAGS) -c $< # 2>> WARNINGS || (cat WARNINGS && false)


gameloop:
	g++ -Wall ${CFLAGS} `sdl-config --cflags --libs` test_gameloop.cpp gameloop.cpp ../iosfc/*.o -o test_gameloop

gui:object
	g++ -Wall -g -O0 ${CFLAGS} `sdl-config --cflags` test_gui.cpp ../iosfc/*.o ${OBJFILES} -lSDL_image -lSDL_mixer `sdl-config --libs` -o test_gui


clean:
	rm -f *.o *~
