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

class Frame : public VBox {
public:
    Frame(IIM_Surface *frameSurface);
    virtual ~Frame();
    virtual void draw(SDL_Surface *screen);
private:
    static const int leftW, middleW, rightW;
    static const int topH, middleH, bottomH;
    IIM_Surface *m_frameSurface;
    SDL_Surface *m_bgSurface;
};

const int Frame::leftW = 25, Frame::middleW = 28, Frame::rightW = 25;
const int Frame::topH = 19, Frame::middleH = 26, Frame::bottomH = 23;

Frame::Frame(IIM_Surface *frameSurface) : m_frameSurface(frameSurface), m_bgSurface(NULL)
{
    setPolicy(USE_MIN_SIZE);
}

Frame::~Frame()
{}

void Frame::draw(SDL_Surface *screen)
{
    Vec3 bsize = getSize();
    SDL_Rect srcrect, dstrect;
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.h = bsize.y;
    srcrect.w = bsize.x;
    
    dstrect.x = getPosition().x;
    dstrect.y = getPosition().y;
    dstrect.h = bsize.y;
    dstrect.w = bsize.x;
    
    // If the background of the frame has not been created or has changed size, recreate it
    if ((m_bgSurface == NULL) || (bsize.x != m_bgSurface->w) || (bsize.y != m_bgSurface->h)) {
        if (m_bgSurface != NULL)
            SDL_FreeSurface(m_bgSurface);
        Uint32 rmask, gmask, bmask, amask;
        /* SDL interprets each pixel as a 32-bit number, so our masks must depend
           on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
        m_bgSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, bsize.x, bsize.y, 32, 
                                      rmask, gmask, bmask, amask);
        SDL_SetAlpha(m_frameSurface->surf, 0, SDL_ALPHA_OPAQUE);
        // Draw the corners first
        // Top left corner
        IIM_Rect src_rect = {0, 0, leftW, topH};
        SDL_Rect dst_rect = {0, 0, leftW, topH};
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Top right corner
        src_rect.x = leftW + middleW; src_rect.y = 0; src_rect.w = rightW; src_rect.h = topH;
        dst_rect.x = bsize.x - rightW; dst_rect.y = 0; dst_rect.w = rightW; dst_rect.h = topH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Bottom left corner
        src_rect.x = 0; src_rect.y = topH + middleH; src_rect.w = leftW; src_rect.h = bottomH;
        dst_rect.x = 0; dst_rect.y = bsize.y - bottomH; dst_rect.w = leftW; dst_rect.h = bottomH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Bottom right corner
        src_rect.x = leftW + middleW; src_rect.y = topH + middleH; src_rect.w = rightW; src_rect.h = bottomH;
        dst_rect.x = bsize.x - rightW; dst_rect.y = bsize.y - bottomH; dst_rect.w = rightW; dst_rect.h = bottomH;
        IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        // Top edge
        src_rect.x = leftW; src_rect.y = 0; src_rect.w = middleW; src_rect.h = topH;
        dst_rect.y = 0; dst_rect.w = middleW; dst_rect.h = topH;
        for (dst_rect.x = leftW ; dst_rect.x < bsize.x - rightW ; dst_rect.x += middleW) {
            if (dst_rect.x + middleW > bsize.x - rightW)
                dst_rect.w = src_rect.w = bsize.x - rightW - dst_rect.x;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Bottom edge
        src_rect.x = leftW; src_rect.y = topH + middleH; src_rect.w = middleW; src_rect.h = bottomH;
        dst_rect.y = bsize.y - bottomH; dst_rect.w = middleW; dst_rect.h = bottomH;
        for (dst_rect.x = leftW ; dst_rect.x < bsize.x - rightW ; dst_rect.x += middleW) {
            if (dst_rect.x + middleW > bsize.x - rightW)
                dst_rect.w = src_rect.w = bsize.x - rightW - dst_rect.x;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Left edge
        src_rect.x = 0; src_rect.y = topH; src_rect.w = leftW; src_rect.h = middleH;
        dst_rect.x = 0; dst_rect.w = leftW; dst_rect.h = middleH;
        for (dst_rect.y = topH ; dst_rect.y < bsize.y - bottomH ; dst_rect.y += middleH) {
            if (dst_rect.y + middleH > bsize.y - bottomH)
                dst_rect.h = src_rect.h = bsize.y - bottomH - dst_rect.y;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Right edge
        src_rect.x = leftW + middleW; src_rect.y = topH; src_rect.w = rightW; src_rect.h = middleH;
        dst_rect.x = bsize.x - rightW; dst_rect.w = rightW; dst_rect.h = middleH;
        for (dst_rect.y = topH ; dst_rect.y < bsize.y - bottomH ; dst_rect.y += middleH) {
            if (dst_rect.y + middleH > bsize.y - bottomH)
                dst_rect.h = src_rect.h = bsize.y - bottomH - dst_rect.y;
            IIM_BlitSurface(m_frameSurface, &src_rect, m_bgSurface, &dst_rect);
        }
        // Content rect
        src_rect.x = leftW; src_rect.y = topH; src_rect.w = bsize.x - leftW - rightW; src_rect.h = bsize.y - topH - bottomH;
        SDL_FillRect(m_bgSurface, &src_rect, m_bgSurface->format->Amask & 0x80808080);
    }
    // Drawing the background
    SDL_BlitSurface(m_bgSurface, &srcrect, screen, &dstrect);
    VBox::draw(screen);
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
    
    Frame frame(IIM_Load_Absolute_DisplayFormatAlpha("data/base.000/gfx/frame.png"));
    //frame.setPreferedSize(Vec3(100, 100));
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
