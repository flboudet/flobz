#include "gameui.h"
using namespace gameui;


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
      setSurface(display);
      add(new ExitComponent());
    }
};


class MainMenuScreen : public ScreenVBox
{
  public:
    MainMenuScreen() : ScreenVBox(0,0,640,480)
    {
      setBackground(IIM_Load_DisplayFormat("Background.jpg"));

      // add some title
      for (int i=1; i<=2; ++i)
        add(new Button(String("FloboPuyo #") + i));
      
      HBox *box1 = new HBox;
      VBox *box2 = new VBox;
      add(box1);
      box1->add(new Button("Left"));
      box1->add(new Button("Middle"));
      box1->add(box2);
      box2->add(new Button("Right-Up"));
      box2->add(new Button("Right-Down"));
      
      for (int i=6; i<=7; ++i)
        add(new Button(String("FloboPuyo #") + i));
    }
};


int main()
{
  // Init SDL
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);

  // Init gametools globals...
  display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF);
  dataFolder = "../data";
 
  // Create the loop 
  PuyoLoop loop;

  // Init a few default values for the GUI
  SoFont_load(GameUIDefaults::FONT,          IIM_Load_DisplayFormatAlpha("font3b.png"));
  SoFont_load(GameUIDefaults::FONT_INACTIVE, IIM_Load_DisplayFormatAlpha("fontdark.png"));
  GameUIDefaults::GAME_LOOP = &loop;

  // Create the GUI
  MainMenuScreen mainMenu;

  // Launch the loop.
  loop.add(&mainMenu);
  loop.run();

  return 0;
}

