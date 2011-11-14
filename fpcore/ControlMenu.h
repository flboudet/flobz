#ifndef FLOBO_CONTROL_MENU_H
#define FLOBO_CONTROL_MENU_H

#include "gameui.h"
#include "MainScreen.h"
#include "PuyoCommander.h"
#include "Frame.h"
#include "FramedButton.h"

class ControlMenu : public MainScreenMenu {
public:
    ControlMenu(MainScreen *mainScreen);
    void build();
private:
    class BackSaveAction : public Action
    {
    public:
        BackSaveAction(MainScreen *mainScreen) : mainScreen(mainScreen) {}
        void action();
    private:
        MainScreen *mainScreen;
    };
    Frame screenTitleFrame;
    Frame playerOneTitleFrame, playerTwoTitleFrame;
    Frame playerOneFrame, playerTwoFrame;
    Text title, playerOneTitle, playerTwoTitle;
    Separator titleSeparator, playersSeparator, bottomSeparator;
    BackSaveAction backAction;
    FramedButton backButton;
    HBox playerOneBox, playerTwoBox;
    VBox label1pBox, key1pBox, alternateKey1pBox;
    VBox label2pBox, key2pBox, alternateKey2pBox;
    Text label1pTitle, key1pTitle, alternateKey1pTitle;
    Text label2pTitle, key2pTitle, alternateKey2pTitle;
    Text lbl1PLeft, lbl1PRight, lbl1PDown, lbl1PTurnRight, lbl1PTurnLeft;
    Text lbl2PLeft, lbl2PRight, lbl2PDown, lbl2PTurnRight, lbl2PTurnLeft;
    ControlInputWidget p1Left, p1Right, p1Down, p1TurnRight, p1TurnLeft;
    ControlInputWidget ap1Left, ap1Right, ap1Down, ap1TurnRight, ap1TurnLeft;
    ControlInputWidget p2Left, p2Right, p2Down, p2TurnRight, p2TurnLeft;
    ControlInputWidget ap2Left, ap2Right, ap2Down, ap2TurnRight, ap2TurnLeft;
};

#endif
