#include "PuyoControlMenu.h"
#include "preferences.h"
#include "PuyoStrings.h"

ControlMenu::ControlMenu(PuyoRealMainScreen *mainScreen) : PuyoMainScreenMenu(mainScreen)
{
}

void ControlMenu::build() {
    add(new Text("Controls"));
    add(new Button("Back", new PuyoPopMenuAction(mainScreen)));
}
