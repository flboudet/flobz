#include "gameui.h"

namespace gameui {

  GameUIEnum GameUIDefaults::CONTAINER_POLICY = USE_MAX_SIZE;
  float      GameUIDefaults::SPACING          = 16.0f;
  SoFont    *GameUIDefaults::FONT          = SoFont_new();
  SoFont    *GameUIDefaults::FONT_INACTIVE = SoFont_new();
  GameLoop  *GameUIDefaults::GAME_LOOP     = NULL;

  //
  // Widget
  // 
  Widget::Widget(WidgetContainer *parent)
    : parent(parent), preferedSize(0,0,0), size(0,0,0),
    position(0,0,0), hidden(false), focus(false)
    {}

  void Widget::draw(SDL_Surface *screen, bool force) const
  {
    if ((force || drawRequested()) && !hidden)
      draw(screen);
  }
  
  void Widget::eventOccured(GameControlEvent *event)
  {
    lostFocus();
  }

  //
  // WidgetContainer
  // 

  WidgetContainer::WidgetContainer(GameLoop *loop) : loop(loop) {
    if (loop == NULL) this->loop = GameUIDefaults::GAME_LOOP;
  }

  void WidgetContainer::add (Widget *child)    
  { 
    childs.add(child);
    child->setParent(this);
    if (child->getIdleComponent()) {
      loop->add(child->getIdleComponent());
    }
    arrangeWidgets();
  }

  void WidgetContainer::remove (Widget *child)
  {
    childs.remove(child);
    if (child->getIdleComponent()) {
      child->getIdleComponent()->kill();
    }
    arrangeWidgets();
  }

  void WidgetContainer::draw(SDL_Surface *surface) const 
  {
    for (int i = 0; i < getNumberOfChilds(); ++i) {
      getChild(i)->draw(surface);
    }
  }

  GameLoop *WidgetContainer::getGameLoop()
  {
    return loop;
  }

  bool WidgetContainer::bubbleSortZ_iteration(int itNumber)
  {
    // Remark: you may think that a bubble sort is a somehow slow sorting
    // algorithm, but in fact this the fastest possible on an already sorted array O(n)
    // In our case, the widget list will not change often: so bubble sort is a good choice.

    if (itNumber < 1) IOS_ERROR("Wrong bubbleSortZ_iteration itNumber\n");

    bool changed = false;

    for (int i = 0; i < childs.size() - itNumber; ++i) {
      Vec3 v1 = childs[i]->getPosition();
      Vec3 v2 = childs[i+1]->getPosition();
      if (v1.z > v2.z) {
        Widget *tmp = childs[i];
        childs[i] = childs[i+1];
        childs[i+1] = tmp;
        changed = true;
      }
    }
    return changed;
  }

  void WidgetContainer::sortWidgets()
  {
    int it = 1;
    // Sort widgets
    while (bubbleSortZ_iteration(it++));
  }

  void WidgetContainer::setSize(const Vec3 &v3)
  {
    Widget::setSize(v3);
    arrangeWidgets();
  }
  
  void WidgetContainer::setPosition(const Vec3 &v3)
  {
    Widget::setPosition(v3);
    arrangeWidgets();
  }

  // 
  // Box
  // 
  Box::Box(GameLoop *loop) : WidgetContainer(loop)
  {
    setPolicy(GameUIDefaults::CONTAINER_POLICY);
  }

  void Box::setPolicy(GameUIEnum policy)
  {
    this->policy = policy;
    arrangeWidgets();
  }

  void Box::arrangeWidgets()
  {
    if (getNumberOfChilds() == 0) return;

    switch (policy)
    {
      case USE_MAX_SIZE:
        {
          float height = getSortingAxe(getSize());
          float heightPerChild = (height + GameUIDefaults::SPACING) / getNumberOfChilds()
            - GameUIDefaults::SPACING;
          Vec3 size     = getSize();
          setSortingAxe(size, heightPerChild);
          Vec3 position = getPosition();
          float axePos  = getSortingAxe(position);

          for (int i = 0; i < getNumberOfChilds(); ++i) {
            Widget *child = getChild(i);
            if (!child->getPreferedSize().is_zero()) {
              // center the widget if we know its size
              Vec3 csize = size - child->getPreferedSize();
              Vec3 cpos  = position + csize / 2.0;
              child->setSize(child->getPreferedSize());
              child->setPosition(cpos);
            }
            else {
              // else give him all the space he want.
              child->setSize(size);
              child->setPosition(position);
            }
            axePos += heightPerChild + GameUIDefaults::SPACING;
            setSortingAxe(position, axePos);
          }
        }
        break;
      case USE_MIN_SIZE:
        {
        }
        break;
    }
  }
  

  //
  // ScreenVBox
  //
  
  ScreenVBox::ScreenVBox(float x, float y, float width, float height, GameLoop *loop) : VBox(loop)
  {
    setPosition(Vec3(x, y, 1.0f));
    setSize(Vec3(width, height, 1.0f));
  }

  void ScreenVBox::setBackground(IIM_Surface *bg)
  {
    this->bg = bg;
  }

  void ScreenVBox::draw(SDL_Surface *surface) const
  {
    if (bg) {
      SDL_Rect rect;
      rect.x = (Sint16)getPosition().x;
      rect.y = (Sint16)getPosition().y;
      rect.w = (Uint16)getSize().x;
      rect.h = (Uint16)getSize().y;
      SDL_BlitSurface(bg->surf, NULL, surface, &rect);
    }
    VBox::draw(surface);
  }
  
  void ScreenVBox::onEvent(GameControlEvent *event)
  {
    VBox::eventOccured(event);
  }

  //
  // Text
  // 
  Text::Text(const String &label, SoFont *font)
    : font(font), label(label)
  {
    if (font == NULL) this->font = GameUIDefaults::FONT;
    setPreferedSize(Vec3(SoFont_TextWidth(this->font, label), SoFont_FontHeight(this->font), 1.0));
  }

  void Text::draw(SDL_Surface *screen) const
  {
    SoFont_PutString(font, screen, (int)getPosition().x, (int)getPosition().y, (const char*)label, NULL);
  }


  Separator::Separator(float width, float height)
  {
    setPreferedSize(Vec3(width, height, 1.0));
  }

};

