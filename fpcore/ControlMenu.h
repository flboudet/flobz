#ifndef FLOBO_CONTROL_MENU_H
#define FLOBO_CONTROL_MENU_H

#include "gameui.h"
#include "MainScreen.h"
#include "FPCommander.h"
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
        BackSaveAction(MainScreen *mainScreen) : _mainScreen(mainScreen) {}
        void action();
    private:
        MainScreen *_mainScreen;
    };
    Frame _screenTitleFrame;
    Frame _playerOneTitleFrame, _playerTwoTitleFrame;
    Frame _playerOneFrame, _playerTwoFrame;
    Text _title, _playerOneTitle, _playerTwoTitle;
    Separator _titleSeparator, _playersSeparator, _bottomSeparator;
    BackSaveAction _backAction;
    FramedButton _backButton;
    HBox _playerOneBox, _playerTwoBox;
    VBox _label1pBox, _key1pBox, _alternateKey1pBox;
    VBox _label2pBox, _key2pBox, _alternateKey2pBox;
    Text _label1pTitle, _key1pTitle, _alternateKey1pTitle;
    Text _label2pTitle, _key2pTitle, _alternateKey2pTitle;
    Text _lbl1PLeft, _lbl1PRight, _lbl1PDown, _lbl1PTurnRight, _lbl1PTurnLeft;
    Text _lbl2PLeft, _lbl2PRight, _lbl2PDown, _lbl2PTurnRight, _lbl2PTurnLeft;
    ControlInputWidget _p1Left, _p1Right, _p1Down, _p1TurnRight, _p1TurnLeft;
    ControlInputWidget _ap1Left, _ap1Right, _ap1Down, _ap1TurnRight, _ap1TurnLeft;
    ControlInputWidget _p2Left, _p2Right, _p2Down, _p2TurnRight, _p2TurnLeft;
    ControlInputWidget _ap2Left, _ap2Right, _ap2Down, _ap2TurnRight, _ap2TurnLeft;
};

#endif
