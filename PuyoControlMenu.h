#ifndef PUYO_CONTROL_MENU_H
#define PUYO_CONTROL_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"

class ControlMenu : public PuyoMainScreenMenu {
public:
    ControlMenu(PuyoRealMainScreen *mainScreen);
    void build();
private:
    Text title;
    Button backButton;
    HBox mainHBox;
    VBox labelBox, keyBox, alternateKeyBox;
    Text labelTitle, keyTitle, alternateKeyTitle;
    Text lbl1PLeft, lbl1PRight, lbl1PDown, lbl1PTurnRight, lbl1PTurnLeft;
    ControlInputWidget p1Left, p1Right, p1Down, p1TurnRight, p1TurnLeft;
    ControlInputWidget ap1Left, ap1Right, ap1Down, ap1TurnRight, ap1TurnLeft;
};

#endif
