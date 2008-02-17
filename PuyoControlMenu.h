#ifndef PUYO_CONTROL_MENU_H
#define PUYO_CONTROL_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"

class ControlMenu : public PuyoMainScreenMenu {
public:
    ControlMenu(PuyoMainScreen *mainScreen);
    void build();
private:
    class BackSaveAction : public Action
    {
    public:
        BackSaveAction(PuyoMainScreen *mainScreen) : mainScreen(mainScreen) {}
        void action();
    private:
        PuyoMainScreen *mainScreen;
    };
    Frame screenTitleFrame;
    Text title;
    BackSaveAction backAction;
    Button backButton;
    HBox mainHBox;
    VBox labelBox, keyBox, alternateKeyBox;
    Text labelTitle, keyTitle, alternateKeyTitle, labelSep1, keySep1, altSep1, labelSep2, keySep2, altSep2;
    Text lbl1PLeft, lbl1PRight, lbl1PDown, lbl1PTurnRight, lbl1PTurnLeft;
    Text lbl2PLeft, lbl2PRight, lbl2PDown, lbl2PTurnRight, lbl2PTurnLeft;
    ControlInputWidget p1Left, p1Right, p1Down, p1TurnRight, p1TurnLeft;
    ControlInputWidget ap1Left, ap1Right, ap1Down, ap1TurnRight, ap1TurnLeft;
    ControlInputWidget p2Left, p2Right, p2Down, p2TurnRight, p2TurnLeft;
    ControlInputWidget ap2Left, ap2Right, ap2Down, ap2TurnRight, ap2TurnLeft;
};

#endif
