#include <stdlib.h>
#include <string.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SDL.h>
#include <SDL_main.h>
#include <vector>

#include "gameui.h"
#include "GameCursor.h"
#include "ListView.h"
#include "IosImgProcess.h"

using namespace ios_fc;
using namespace gameui;

extern "C"

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
    virtual void action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
    {
        printf("Clic sur %x\n", sender);
    }
};

Screen *pscr;
    
class ShowModalDialogAction : public Action {
public:
    ShowModalDialogAction(ZBox *rootZBox);
    virtual void action(Widget *sender, GameUIEnum actionType, GameControlEvent *event);
private:
    ZBox *rootZBox;
    SliderContainer slider;
    VBox dlgBox;
    Text title;
    Button simpleButton;
};

ShowModalDialogAction::ShowModalDialogAction(ZBox *rootZBox) : rootZBox(rootZBox), title("Slider"), simpleButton("button")
{
    IIM_Surface *sliderBkgnd = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/menubg.png");
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

void ShowModalDialogAction::action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
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
    
    Button bidonButton1("Hi", &showDialogAction);
    Button bidonButton2("Ho");
    Button bidonButton3("Hop");
    IIM_Surface *upArrow = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/uparrow.png");
    IIM_Surface *downArrow = IIM_Load_Absolute_DisplayFormatAlpha ("data/base.000/gfx/downarrow.png");
    bidonBox.setPolicy(USE_MIN_SIZE);
    ListView list(20, upArrow, downArrow);
    bidonBox.add(&bidonText);
    bidonBox.add(&bidonButton1);
    bidonBox.add(&bidonButton2);
    for (int i = 0 ; i < 100 ; i++) {
        String newEntry("Bla bla ");
        list.addEntry(new ListViewEntry(newEntry + i, &actionBidon));
    }
    bidonBox.add(&list);
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
