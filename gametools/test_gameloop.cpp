#include "gameloop.h"

class A : public CycledComponent, DrawableComponent
{
  public:
  A() : CycledComponent(1.0) {}
  
  void cycle() {
    printf("A::cycle %d\n", getCycleNumber());

    if (getCycleNumber() % 2) {
      requestDraw();
    }
  }

  void draw() {
    printf("A::draw\n");
  }
};

int main()
{
  GameLoop loop;
  loop.add(new A);
  loop.run();
}
