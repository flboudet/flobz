#include "ControlMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"

using namespace event_manager;

void ControlMenu::BackSaveAction::action()
{
    GameUIDefaults::GAME_LOOP->getEventManager()->saveControls();
    mainScreen->popMenu();
}

ControlMenu::ControlMenu(MainScreen *mainScreen)
    : MainScreenMenu(mainScreen),
      screenTitleFrame(theCommander->getSeparatorFramePicture()),
      playerOneTitleFrame(theCommander->getSeparatorFramePicture()),
      playerTwoTitleFrame(theCommander->getSeparatorFramePicture()),
      playerOneFrame(theCommander->getWindowFramePicture()),
      playerTwoFrame(theCommander->getWindowFramePicture()),
      title(theCommander->getLocalizedString("Change Controls")),
      playerOneTitle(theCommander->getLocalizedString("Player 1 Controls")),
      playerTwoTitle(theCommander->getLocalizedString("Player 2 Controls")),
      titleSeparator(0., 10.), playersSeparator(0., 10.), bottomSeparator(0., 10.),
      backAction(mainScreen),
      backButton(theCommander->getLocalizedString("Back"), &backAction,
		 theCommander->getButtonFramePicture(),
		 theCommander->getButtonOverFramePicture()),
      label1pTitle(theCommander->getLocalizedString("Controls")),
      key1pTitle(theCommander->getLocalizedString("Primary")),
      alternateKey1pTitle(theCommander->getLocalizedString("Alternate")),
      label2pTitle(theCommander->getLocalizedString("Controls")),
      key2pTitle(theCommander->getLocalizedString("Primary")),
      alternateKey2pTitle(theCommander->getLocalizedString("Alternate")),
      lbl1PLeft(theCommander->getLocalizedString(kSPlayer1Left)),
      lbl1PRight(theCommander->getLocalizedString(kSPlayer1Right)),
      lbl1PDown(theCommander->getLocalizedString(kSPlayer1Down)),
      lbl1PTurnRight(theCommander->getLocalizedString(kSPlayer1Clockwise)),
      lbl1PTurnLeft(theCommander->getLocalizedString(kSPlayer1Counterclockwise)),
      lbl2PLeft(theCommander->getLocalizedString(kSPlayer2Left)),
      lbl2PRight(theCommander->getLocalizedString(kSPlayer2Right)),
      lbl2PDown(theCommander->getLocalizedString(kSPlayer2Down)),
      lbl2PTurnRight(theCommander->getLocalizedString(kSPlayer2Clockwise)),
      lbl2PTurnLeft(theCommander->getLocalizedString(kSPlayer2Counterclockwise)),
      p1Left(0, false), p1Right(1, false), p1Down(2, false), p1TurnRight(3, false), p1TurnLeft(4, false),
      ap1Left(0, true), ap1Right(1, true), ap1Down(2, true), ap1TurnRight(3, true), ap1TurnLeft(4, true),
      p2Left(5, false), p2Right(6, false), p2Down(7, false), p2TurnRight(8, false), p2TurnLeft(9, false),
      ap2Left(5, true), ap2Right(6, true), ap2Down(7, true), ap2TurnRight(8, true), ap2TurnLeft(9, true)
{
    lbl1PLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PDown.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PTurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PTurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);

    lbl2PLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl2PRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl2PDown.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl2PTurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl2PTurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);

    this->setBorderVisible(false);
}

void ControlMenu::build() {
    setPolicy(USE_MIN_SIZE);
    screenTitleFrame.setPreferedSize(Vec3(0, 20));
    screenTitleFrame.add(&title);
    add(&screenTitleFrame);
    add(&titleSeparator);

    playerOneTitleFrame.setPreferedSize(Vec3(0, 20));
    playerTwoTitleFrame.setPreferedSize(Vec3(0, 20));
    playerOneTitleFrame.add(&playerOneTitle);
    playerTwoTitleFrame.add(&playerTwoTitle);

    label1pBox.add(&label1pTitle);
    key1pBox.add(&key1pTitle);
    alternateKey1pBox.add(&alternateKey1pTitle);

    label2pBox.add(&label2pTitle);
    key2pBox.add(&key2pTitle);
    alternateKey2pBox.add(&alternateKey2pTitle);

    label1pBox.add(&lbl1PLeft);
    label1pBox.add(&lbl1PRight);
    label1pBox.add(&lbl1PDown);
    label1pBox.add(&lbl1PTurnRight);
    label1pBox.add(&lbl1PTurnLeft);

    key1pBox.add(&p1Left);
    key1pBox.add(&p1Right);
    key1pBox.add(&p1Down);
    key1pBox.add(&p1TurnRight);
    key1pBox.add(&p1TurnLeft);

    alternateKey1pBox.add(&ap1Left);
    alternateKey1pBox.add(&ap1Right);
    alternateKey1pBox.add(&ap1Down);
    alternateKey1pBox.add(&ap1TurnRight);
    alternateKey1pBox.add(&ap1TurnLeft);

    label2pBox.add(&lbl2PLeft);
    label2pBox.add(&lbl2PRight);
    label2pBox.add(&lbl2PDown);
    label2pBox.add(&lbl2PTurnRight);
    label2pBox.add(&lbl2PTurnLeft);

    key2pBox.add(&p2Left);
    key2pBox.add(&p2Right);
    key2pBox.add(&p2Down);
    key2pBox.add(&p2TurnRight);
    key2pBox.add(&p2TurnLeft);

    alternateKey2pBox.add(&ap2Left);
    alternateKey2pBox.add(&ap2Right);
    alternateKey2pBox.add(&ap2Down);
    alternateKey2pBox.add(&ap2TurnRight);
    alternateKey2pBox.add(&ap2TurnLeft);

    playerOneBox.add(&label1pBox);
    playerOneBox.add(&key1pBox);
    playerOneBox.add(&alternateKey1pBox);

    playerTwoBox.add(&label2pBox);
    playerTwoBox.add(&key2pBox);
    playerTwoBox.add(&alternateKey2pBox);

    playerOneFrame.add(&playerOneBox);
    playerTwoFrame.add(&playerTwoBox);
    add(&playerOneTitleFrame);
    add(&playerOneFrame);
    add(&playersSeparator);
    add(&playerTwoTitleFrame);
    add(&playerTwoFrame);
    add(&bottomSeparator);
    add(&backButton);
}
