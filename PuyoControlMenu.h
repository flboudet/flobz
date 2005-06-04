#ifndef PUYO_CONTROL_MENU_H
#define PUYO_CONTROL_MENU_H

#include "gameui.h"
#include "PuyoCommander.h"

class ControlMenu : public PuyoMainScreenMenu {
public:
    ControlMenu(PuyoRealMainScreen *mainScreen);
    void build();
private:
/*    ToggleSoundFxAction toggleSoundFxAction;
    ToggleMusicAction toggleMusicAction;
    ToggleFullScreenAction toggleFullScreenAction;
    ToggleButton audioButton, musicButton, fullScreenButton;
    Button changeControlsButton;*/
};

#endif
