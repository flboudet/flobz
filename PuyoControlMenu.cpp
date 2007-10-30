#include "PuyoControlMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"



void ControlMenu::BackSaveAction::action()
{
    saveControls();
    mainScreen->popMenu();
}

ControlMenu::ControlMenu(PuyoMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), title("Change Controls"), backAction(mainScreen), backButton("Back", &backAction),
      labelTitle("Controls"), keyTitle("Primary"), alternateKeyTitle("Alternate"),
      lbl1PLeft(kPlayer1Left), lbl1PRight(kPlayer1Right), lbl1PDown(kPlayer1Down), lbl1PTurnRight(kPlayer1Clockwise), lbl1PTurnLeft(kPlayer1Counterclockwise),
      lbl2PLeft(kPlayer2Left), lbl2PRight(kPlayer2Right), lbl2PDown(kPlayer2Down), lbl2PTurnRight(kPlayer2Clockwise), lbl2PTurnLeft(kPlayer2Counterclockwise),
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
}

void ControlMenu::build() {
    labelBox.add(&labelTitle);
    keyBox.add(&keyTitle);
    alternateKeyBox.add(&alternateKeyTitle);
    
    labelBox.add(&labelSep1);
    keyBox.add(&keySep1);
    alternateKeyBox.add(&altSep1);
    
    labelBox.add(&lbl1PLeft);
    labelBox.add(&lbl1PRight);
    labelBox.add(&lbl1PDown);
    labelBox.add(&lbl1PTurnRight);
    labelBox.add(&lbl1PTurnLeft);
    
    keyBox.add(&p1Left);
    keyBox.add(&p1Right);
    keyBox.add(&p1Down);
    keyBox.add(&p1TurnRight);
    keyBox.add(&p1TurnLeft);
    
    alternateKeyBox.add(&ap1Left);
    alternateKeyBox.add(&ap1Right);
    alternateKeyBox.add(&ap1Down);
    alternateKeyBox.add(&ap1TurnRight);
    alternateKeyBox.add(&ap1TurnLeft);
    
    labelBox.add(&labelSep2);
    keyBox.add(&keySep2);
    alternateKeyBox.add(&altSep2);
    
    labelBox.add(&lbl2PLeft);
    labelBox.add(&lbl2PRight);
    labelBox.add(&lbl2PDown);
    labelBox.add(&lbl2PTurnRight);
    labelBox.add(&lbl2PTurnLeft);
    
    keyBox.add(&p2Left);
    keyBox.add(&p2Right);
    keyBox.add(&p2Down);
    keyBox.add(&p2TurnRight);
    keyBox.add(&p2TurnLeft);
    
    alternateKeyBox.add(&ap2Left);
    alternateKeyBox.add(&ap2Right);
    alternateKeyBox.add(&ap2Down);
    alternateKeyBox.add(&ap2TurnRight);
    alternateKeyBox.add(&ap2TurnLeft);

    mainHBox.add(&labelBox);
    mainHBox.add(&keyBox);
    mainHBox.add(&alternateKeyBox);
    
    Vec3 margins(400.0,400.0,0.0);
    mainHBox.setPreferedSize(/*mainHBox.getPreferedSize() + */margins);
    
    add(&title);
    add(&mainHBox);
    add(&backButton);
}
