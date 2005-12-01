#include "PuyoControlMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"

ControlMenu::ControlMenu(PuyoRealMainScreen *mainScreen)
    : PuyoMainScreenMenu(mainScreen), title("Controls"), backButton("Back", new PuyoPopMenuAction(mainScreen)),
      labelTitle(""), keyTitle("Primary"), alternateKeyTitle("Alternate"),
      lbl1PLeft(kPlayer1Left), lbl1PRight(kPlayer1Right), lbl1PDown(kPlayer1Down), lbl1PTurnRight(kPlayer1Clockwise), lbl1PTurnLeft(kPlayer1Counterclockwise),
      p1Left("A"), p1Right("A"), p1Down("A"), p1TurnRight("A"), p1TurnLeft("A"),
      ap1Left("A"), ap1Right("A"), ap1Down("A"), ap1TurnRight("A"), ap1TurnLeft("A")
{
    lbl1PLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PDown.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PTurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    lbl1PTurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    
    /*p1Left.setFont(GameUIDefaults::FONT_SMALL_INFO);
    p1Right.setFont(GameUIDefaults::FONT_SMALL_INFO);
    p1Down.setFont(GameUIDefaults::FONT_SMALL_INFO);
    p1TurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    p1TurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);
    
    ap1Left.setFont(GameUIDefaults::FONT_SMALL_INFO);
    ap1Right.setFont(GameUIDefaults::FONT_SMALL_INFO);
    ap1Down.setFont(GameUIDefaults::FONT_SMALL_INFO);
    ap1TurnRight.setFont(GameUIDefaults::FONT_SMALL_INFO);
    ap1TurnLeft.setFont(GameUIDefaults::FONT_SMALL_INFO);*/
}

void ControlMenu::build() {
    labelBox.add(&labelTitle);
    keyBox.add(&keyTitle);
    alternateKeyBox.add(&alternateKeyTitle);
    
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

    mainHBox.add(&labelBox);
    mainHBox.add(&keyBox);
    mainHBox.add(&alternateKeyBox);
    
    add(&title);
    add(&mainHBox);
    add(&backButton);
}
