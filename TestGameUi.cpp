#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_main.h>
#include <vector>

#include "gameui.h"
#include "GameCursor.h"
#include "ListView.h"
#include "Frame.h"
#include "FramedButton.h"
#include "FramedEditField.h"
#include "IosImgProcess.h"
#include "SwitchedButton.h"
#include "PuyoStatsWidget.h"

using namespace ios_fc;
using namespace gameui;

extern "C"

PlayerGameStat::PlayerGameStat(int p)
{
    for (int i=0; i<24; ++i)
        combo_count[i] = 0;
    ghost_sent_count = 0;
    time_left = 0;
    is_dead = false;
    is_winner = false;
    points = p;
}

class BusyWidget : public Widget, IdleComponent {
public:
    void idle(double currentTime) { requestDraw(); }
    void draw(SDL_Surface *screen);
    IdleComponent *getIdleComponent() { return this; }
};

void BusyWidget::draw(SDL_Surface *screen)
{
    SDL_Rect dstrect;
    
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.h = 480;
    dstrect.w = 640;
    SDL_FillRect(screen, &dstrect, 0x11111166);
}


class TestAction : public Action {
public:
    virtual void action(Widget *sender, int actionType, GameControlEvent *event)
    {
        printf("Clic sur %p\n", sender);
    }
};

Screen *pscr;


FramePicture *windowFramePict;

class TestPuyoStatsAction : public Action {
public:
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
};

void TestPuyoStatsAction::action(Widget *sender, int actionType, GameControlEvent *event)
{
    PlayerGameStat stats;
    GameUIDefaults::SCREEN_STACK->top()->add(new PuyoStatsWidget(stats, windowFramePict, LEFT_TO_RIGHT));
    //GameUIDefaults::SCREEN_STACK->top()->add(new PuyoStatsWidget(stats, windowFramePict, RIGHT_TO_LEFT));
}

class ShowModalDialogAction : public Action {
public:
    ShowModalDialogAction(ZBox *rootZBox);
    virtual void action(Widget *sender, int actionType, GameControlEvent *event);
private:
    ZBox *rootZBox;
    SliderContainer slider;
    VBox dlgBox;
    Text title;
    Button simpleButton;
};

ShowModalDialogAction::ShowModalDialogAction(ZBox *rootZBox) : rootZBox(rootZBox), title("Slider"), simpleButton("button")
{
    IIM_Surface *sliderBkgnd = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/frame.png");
    slider.setBackground(sliderBkgnd);
    Vec3 dialogPos = slider.getPosition();
    dialogPos.x = 50;
    dialogPos.y = 195;
    slider.setPosition(dialogPos);
    dlgBox.add(&title);
    dlgBox.add(&simpleButton);
    slider.setSize(Vec3(sliderBkgnd->w, sliderBkgnd->h));
    slider.add(&dlgBox);
}

void ShowModalDialogAction::action(Widget *sender, int actionType, GameControlEvent *event)
{
    rootZBox->add(&slider);
    slider.getParentScreen()->grabEventsOnWidget(&slider);
}

int main(int argc, char *argv[])
{
    TestAction actionBidon;

    int init_flags = SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK;
    
    if ( SDL_Init(init_flags) < 0 ) {
        fprintf(stderr, "SDL initialisation error:  %s\n", SDL_GetError());
        exit(1);
    }
    GameLoop *loop = GameUIDefaults::GAME_LOOP;
    SDL_Surface *display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF);
    if ( display == NULL ) {
        fprintf(stderr, "SDL_SetVideoMode error: %s\n",
                SDL_GetError());
        exit(1);
    }
    loop->setSurface(display);
    atexit(SDL_Quit); 
    SDL_ShowCursor(SDL_DISABLE);
    SDL_WM_SetCaption("FloboPuyo by iOS-Software",NULL);
  
    // Font par défaut
    SoFont *darkFont = SoFont_new();
    SoFont *menuFont = SoFont_new();
    SoFont *textFont = SoFont_new();
    SoFont_load_ttf(darkFont, "data/base.000/gfx/font.ttf", 17, SoFont_DARK);
    SoFont_load_ttf(menuFont, "data/base.000/gfx/font.ttf", 17, SoFont_STD);
    SoFont_load_ttf(textFont, "data/base.000/gfx/font.ttf", 24, SoFont_GREY);

    GameUIDefaults::FONT_TEXT       = textFont;
    GameUIDefaults::FONT            = menuFont;
    GameUIDefaults::FONT_INACTIVE   = darkFont;
    GameCursor *cursor = new GameCursor("data/base.000/gfx/cursor.png");
    loop->addDrawable(cursor);
    loop->addIdle(cursor);
    
    Screen scr(0, 0, 640, 480);
    pscr = &scr;
    VBox bidonBox;
    Text bidonText("Hello");
    ShowModalDialogAction showDialogAction(scr.getRootContainer());
    
    FramePicture fpict(IIM_Load_Absolute_DisplayFormatAlpha("data/base.000/gfx/frame.png"),
                       25, 28, 25, 19, 26, 23);
    windowFramePict = &fpict;
    FramePicture fpict2(IIM_Load_Absolute_DisplayFormatAlpha(
		       "data/base.000/gfx/editfield.png"),
			5, 23, 4, 6, 10, 3);
    FramePicture fpict3(IIM_Load_Absolute_DisplayFormatAlpha(
		       "data/base.000/gfx/separator.png"),
			63, 2, 63, 2, 4, 2);
    RGBA blackTranslucient = {(Uint8)0x00, (Uint8)0x00, (Uint8)0x00, (Uint8)0x80};
    RGBA buttonGray = {(Uint8)0xCC, (Uint8)0x95, (Uint8)0x36, (Uint8)0xFF};
    RGBA textFieldColor = {(Uint8)0xFA, (Uint8)0xEF, (Uint8)0xDB, (Uint8)0xFF};
    fpict.setContentColor(blackTranslucient);
    fpict2.setContentColor(textFieldColor);
    fpict3.setContentColor(buttonGray);

    Frame frame(&fpict), frame2(&fpict2);
    frame2.setFocusedPicture(&fpict3);
    //frame.setPreferedSize(Vec3(100, 100));
    FramedButton framedButton1("Hi1", NULL, &fpict2, &fpict3);
    Button bidonButton1("Hi2", &showDialogAction);
    TestPuyoStatsAction statsAction;
    Button bidonButton2("Stats", &statsAction);
    Button bidonButton3("Hop");
    FramedEditField bidonField("toto", NULL, &fpict2, &fpict2);
    IIM_Surface *upArrow = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/uparrow.png");
    IIM_Surface *downArrow = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/downarrow.png");
    bidonBox.setPolicy(USE_MIN_SIZE);
    ListView list(10, upArrow, downArrow, &fpict);
    IIM_Surface *onSwitchImage = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/switch-on.png");
    IIM_Surface *offSwitchImage = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/switch-off.png");
    SwitchedButton prefSwitchA(String("Mon switch tout neuf A"), true,
                              onSwitchImage, offSwitchImage,
                              String("test.widget.switchbuttonAB"));
    SwitchedButton prefSwitchB(String("Mon switch tout neuf B sur la même pref que A"), true,
                               onSwitchImage, offSwitchImage,
                               String("test.widget.switchbuttonAB"));
    SwitchedButton prefSwitchC(String("Mon switch tout neuf C sur une autre pref"), false,
                               onSwitchImage, offSwitchImage,
                               String("test.widget.switchbuttonC"));
    bidonBox.add(&prefSwitchA);
    bidonBox.add(&prefSwitchB);
    bidonBox.add(&prefSwitchC);
    bidonBox.add(&bidonText);
    //bidonBox.add(&framedButton1);
    //bidonBox.add(&bidonButton1);
    bidonBox.add(&bidonButton2);
    //bidonBox.add(&frame2);
    bidonBox.add(&bidonField);
    bidonBox.add(&frame);
    for (int i = 0 ; i < 100 ; i++) {
        String newEntry("Bla bla ");
        list.addEntry(new ListViewEntry(newEntry + i, &actionBidon));
    }
    frame.add(&list);
    //bidonBox.add(&list);
    bidonBox.add(&bidonButton3);
    //list.add(&bidonButton1);
    //list.add(&bidonButton2);
    
    BusyWidget busy;
    scr.add(&busy);
    scr.add(&bidonBox);
    GameUIDefaults::SCREEN_STACK->push(&scr);
    
    GameUIDefaults::GAME_LOOP->run();
    return 0;
}
