#include "gameui.h"
using namespace gameui;

/*
 * Define a new Action
 */
class PrintfAction : public Action {
  public:
    void action() {
      printf("COUCOU !\n");
    }
};

/*
 * Declaration of our menus
 */
class OtherMenuScreen : public Screen {
  public:
    OtherMenuScreen() : Screen(0,0,640,480) {}
    void build();
};

class MainMenuScreen : public Screen
{
  public:
    MainMenuScreen() : Screen(0,0,640,480) {}
    void build();
};

static OtherMenuScreen otherMenu;
static MainMenuScreen  mainMenu;

/* 
 * Definition of our menus
 */
void OtherMenuScreen::build()
{
  setBackground(IIM_Load_DisplayFormat("Background.jpg"));
  add(new Button("Hello World !", new PrintfAction));
  add(new Button("Return",        new PopScreenAction));
}

void MainMenuScreen::build()
{
  setBackground(IIM_Load_DisplayFormat("Background.jpg"));
  add(new Text("FloboPuyo #1"));
  add(new Button("FloboPuyo #2", new PrintfAction));

  HBox *box1 = new HBox;
  VBox *box2 = new VBox;
  add(box1);
  box1->add(new Button("Open Sub-Menu", new PushScreenAction(&otherMenu)));
  box1->add(new Button("Middle"));
  box2->add(new Button("Right-Up"));
  box2->add(new Button("Right-Down"));
  box1->add(box2);

  for (int i=3; i<=6; ++i)
    add(new Button(String("FloboPuyo #") + i));
}

/*
 * Main
 */
int main(int argc, char *argv[])
{
  // Init SDL
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);

  // Init gametools globals...
  display = SDL_SetVideoMode( 640, 480, 0,  SDL_ANYFORMAT|SDL_HWSURFACE|SDL_DOUBLEBUF);
  dataFolder = "../data";
 
  // Init a few default values for the GUI
  SoFont_load(GameUIDefaults::FONT,          IIM_Load_DisplayFormatAlpha("font3b.png"));
  SoFont_load(GameUIDefaults::FONT_INACTIVE, IIM_Load_DisplayFormatAlpha("fontdark.png"));

  // Create the GUI
  mainMenu.build();
  otherMenu.build();

  // Launch the loop.
  GameUIDefaults::SCREEN_STACK->push(&mainMenu);
  GameLoop *loop = GameUIDefaults::GAME_LOOP;
  loop->setSurface(display);
  loop->run();

  return 0;
}

