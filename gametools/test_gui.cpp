#include "gameui.h"
using namespace gameui;

extern SDL_Surface *display;

class ExitComponent : public IdleComponent
{
  void onEvent(GameControlEvent *event)
  {
    if (event->cursorEvent == GameControlEvent::kQuit)
    {
      SDL_Quit();
      exit(0);
    }
  }
};

class PuyoLoop : public GameLoop
{
  public:
    PuyoLoop()
    {
      int init_flags = SDL_INIT_VIDEO|SDL_INIT_JOYSTICK;
      SDL_Init(init_flags);
    
      display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF);
      dataFolder = "../data";
      
      setSurface(display);

      add(new ExitComponent());
    }
};

class MainMenuScreen : public ScreenVBox
{
  public:
    MainMenuScreen(GameLoop *loop) : ScreenVBox(loop, 0,0,640,480)
    {
      setBackground(IIM_Load_DisplayFormat("Background.jpg"));
      // load a font
      SoFont *font = SoFont_new();
      SoFont_load(font, IIM_Load_DisplayFormatAlpha("font3b.png"));
      // add some title
      for (int i=1; i<=10; ++i)
        add(new TextWidget(font, String("FloboPuyo #") + i));
    }
};

int main()
{
    PuyoLoop loop;
    MainMenuScreen mainMenu(&loop);
    loop.add(&mainMenu);
    loop.run();
    return 0;
}

