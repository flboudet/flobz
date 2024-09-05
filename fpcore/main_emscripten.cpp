#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "FPMain.h"

static volatile bool entered = false;

// Our "main loop" function. This callback receives the current time as
// reported by the browser, and the user data we provide in the call to
// emscripten_request_animation_frame_loop().
void one_iter() {
    if (entered)
        return;
    entered = true;
    // Can render to the screen here, etc.

    double currentTime = GameLoop::getCurrentTime();
    GameUIDefaults::GAME_LOOP->idle(currentTime);
    if (GameUIDefaults::GAME_LOOP->drawRequested())
        GameUIDefaults::GAME_LOOP->draw();
    //return GameUIDefaults::GAME_LOOP->drawRequested();
    entered = false;
}

int main() {
    std::cout << "Entering main()\n";
    std::ifstream tst("/data/base.000/gfx/switch-on.png");
    std::cout << "Test file is good: " << tst.good() <<std::endl;
    FPMain *fpmain = new FPMain("/data", false);
    std::cout << "fpmain created\n";
    fpmain->run();
    std::cout << "fpmain initialized\n";
#ifdef __EMSCRIPTEN__
  // Receives a function to call and some user data to provide it.
  //emscripten_set_main_loop(one_iter, 60, false);
  emscripten_set_main_loop(one_iter, 0, 1);
  //emscripten_request_animation_frame_loop(one_iter, 0);
  
  std::cout << "done\n";
  return 0;
#else
  while (1) {
    one_iter();
    // Delay to keep frame rate constant (using SDL).
    SDL_Delay(time_to_next_frame());
  }
#endif
}
