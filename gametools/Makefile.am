noinst_LIBRARIES=libgametools.a

libgametools_a_SOURCES=SDL_Painter.cpp InputManager.cpp GameControls.cpp GameCursor.cpp \
         IosImgProcess.cpp preferences.cpp sofont.cpp gameloop.cpp gameui.cpp \
         ListView.cpp Frame.cpp FramedButton.cpp FramedEditField.cpp \
	 mt19937ar.cpp SwitchedButton.cpp RadioButton.cpp NotifyCenter.cpp

libgametools_a_CXXFLAGS = -I../iosfc $(SDL_CFLAGS) $(AM_CXXFLAGS) $(FPFEATURES)
