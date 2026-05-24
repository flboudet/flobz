#include "ControlMenu.h"
#include "FPStrings.h"

using namespace event_manager;

void ControlMenu::BackSaveAction::action()
{
    GameUIDefaults::GAME_LOOP->getEventManager()->saveControls();
    _mainScreen->popMenu();
}

ControlMenu::ControlMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      _screenTitleFrame(theCommander->getSeparatorFramePicture()),
      _playerOneTitleFrame(theCommander->getSeparatorFramePicture()),
      _playerTwoTitleFrame(theCommander->getSeparatorFramePicture()),
      _playerOneFrame(theCommander->getWindowFramePicture()),
      _playerTwoFrame(theCommander->getWindowFramePicture()),
      _title(theCommander->getLocalizedString("Change Controls")),
      _playerOneTitle(theCommander->getLocalizedString("Player 1 Controls")),
      _playerTwoTitle(theCommander->getLocalizedString("Player 2 Controls")),
      _titleSeparator(0., 10.), _playersSeparator(0., 10.), _bottomSeparator(0., 10.),
      _backAction(mainScreen),
      _backButton(theCommander->getLocalizedString("Back"), &_backAction,
		 theCommander->getButtonFramePicture(),
		 theCommander->getButtonOverFramePicture()),
      _label1pTitle(theCommander->getLocalizedString("Controls")),
      _key1pTitle(theCommander->getLocalizedString("Primary")),
      _alternateKey1pTitle(theCommander->getLocalizedString("Alternate")),
      _label2pTitle(theCommander->getLocalizedString("Controls")),
      _key2pTitle(theCommander->getLocalizedString("Primary")),
      _alternateKey2pTitle(theCommander->getLocalizedString("Alternate")),
      _lbl1PLeft(theCommander->getLocalizedString(kSPlayer1Left)),
      _lbl1PRight(theCommander->getLocalizedString(kSPlayer1Right)),
      _lbl1PDown(theCommander->getLocalizedString(kSPlayer1Down)),
      _lbl1PTurnRight(theCommander->getLocalizedString(kSPlayer1Clockwise)),
      _lbl1PTurnLeft(theCommander->getLocalizedString(kSPlayer1Counterclockwise)),
      _lbl2PLeft(theCommander->getLocalizedString(kSPlayer2Left)),
      _lbl2PRight(theCommander->getLocalizedString(kSPlayer2Right)),
      _lbl2PDown(theCommander->getLocalizedString(kSPlayer2Down)),
      _lbl2PTurnRight(theCommander->getLocalizedString(kSPlayer2Clockwise)),
      _lbl2PTurnLeft(theCommander->getLocalizedString(kSPlayer2Counterclockwise)),
      _p1Left(0, false), _p1Right(1, false), _p1Down(2, false), _p1TurnRight(3, false), _p1TurnLeft(4, false),
      _ap1Left(0, true), _ap1Right(1, true), _ap1Down(2, true), _ap1TurnRight(3, true), _ap1TurnLeft(4, true),
      _p2Left(5, false), _p2Right(6, false), _p2Down(7, false), _p2TurnRight(8, false), _p2TurnLeft(9, false),
      _ap2Left(5, true), _ap2Right(6, true), _ap2Down(7, true), _ap2TurnRight(8, true), _ap2TurnLeft(9, true)
{
    _lbl1PLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl1PRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl1PDown.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl1PTurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl1PTurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);

    _lbl2PLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl2PRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl2PDown.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl2PTurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    _lbl2PTurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);

    this->setBorderVisible(false);
}

void ControlMenu::build() {
    setPolicy(USE_MIN_SIZE);
    _screenTitleFrame.setPreferedSize(Vec3(0, 20));
    _screenTitleFrame.add(&_title);
    add(&_screenTitleFrame);
    add(&_titleSeparator);

    _playerOneTitleFrame.setPreferedSize(Vec3(0, 20));
    _playerTwoTitleFrame.setPreferedSize(Vec3(0, 20));
    _playerOneTitleFrame.add(&_playerOneTitle);
    _playerTwoTitleFrame.add(&_playerTwoTitle);

    _label1pBox.add(&_label1pTitle);
    _key1pBox.add(&_key1pTitle);
    _alternateKey1pBox.add(&_alternateKey1pTitle);

    _label2pBox.add(&_label2pTitle);
    _key2pBox.add(&_key2pTitle);
    _alternateKey2pBox.add(&_alternateKey2pTitle);

    _label1pBox.add(&_lbl1PLeft);
    _label1pBox.add(&_lbl1PRight);
    _label1pBox.add(&_lbl1PDown);
    _label1pBox.add(&_lbl1PTurnRight);
    _label1pBox.add(&_lbl1PTurnLeft);

    _key1pBox.add(&_p1Left);
    _key1pBox.add(&_p1Right);
    _key1pBox.add(&_p1Down);
    _key1pBox.add(&_p1TurnRight);
    _key1pBox.add(&_p1TurnLeft);

    _alternateKey1pBox.add(&_ap1Left);
    _alternateKey1pBox.add(&_ap1Right);
    _alternateKey1pBox.add(&_ap1Down);
    _alternateKey1pBox.add(&_ap1TurnRight);
    _alternateKey1pBox.add(&_ap1TurnLeft);

    _label2pBox.add(&_lbl2PLeft);
    _label2pBox.add(&_lbl2PRight);
    _label2pBox.add(&_lbl2PDown);
    _label2pBox.add(&_lbl2PTurnRight);
    _label2pBox.add(&_lbl2PTurnLeft);

    _key2pBox.add(&_p2Left);
    _key2pBox.add(&_p2Right);
    _key2pBox.add(&_p2Down);
    _key2pBox.add(&_p2TurnRight);
    _key2pBox.add(&_p2TurnLeft);

    _alternateKey2pBox.add(&_ap2Left);
    _alternateKey2pBox.add(&_ap2Right);
    _alternateKey2pBox.add(&_ap2Down);
    _alternateKey2pBox.add(&_ap2TurnRight);
    _alternateKey2pBox.add(&_ap2TurnLeft);

    _playerOneBox.add(&_label1pBox);
    _playerOneBox.add(&_key1pBox);
    _playerOneBox.add(&_alternateKey1pBox);

    _playerTwoBox.add(&_label2pBox);
    _playerTwoBox.add(&_key2pBox);
    _playerTwoBox.add(&_alternateKey2pBox);

    _playerOneFrame.add(&_playerOneBox);
    _playerTwoFrame.add(&_playerTwoBox);
    add(&_playerOneTitleFrame);
    add(&_playerOneFrame);
    add(&_playersSeparator);
    add(&_playerTwoTitleFrame);
    add(&_playerTwoFrame);
    add(&_bottomSeparator);
    add(&_backButton);
}
