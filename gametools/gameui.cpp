#include "gameui.h"

#define DEFAULT_CONTAINER_POLICY USE_MAX_SIZE
#define DEFAULT_SPACING          16.0f

namespace gameui {

  //
  // Widget
  // 
  Widget::Widget(WidgetContainer *parent)
    : parent(parent), preferedSize(0,0,0), size(0,0,0),
    position(0,0,0), hidden(false)
    {}

  void Widget::draw(SDL_Surface *screen, bool force) const
  {
    if ((force || drawRequested()) && !hidden)
      draw(screen);
  }

  //
  // WidgetContainer
  // 

  WidgetContainer::WidgetContainer(GameLoop *loop) : loop(loop) {}

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
  // BoxWidget
  // 
  BoxWidget::BoxWidget(GameLoop *loop) : WidgetContainer(loop)
  {
    setPolicy(DEFAULT_CONTAINER_POLICY);
  }

  void BoxWidget::setPolicy(GameUIEnum policy)
  {
    this->policy = policy;
    arrangeWidgets();
  }

  void BoxWidget::arrangeWidgets()
  {
    if (getNumberOfChilds() == 0) return;

    switch (policy)
    {
      case USE_MAX_SIZE:
        {
          float height = getSortingAxe(getSize());
          float heightPerChild = (height + DEFAULT_SPACING) / getNumberOfChilds() - DEFAULT_SPACING;
          Vec3 size     = getSize();
          setSortingAxe(size, heightPerChild);
          Vec3 position = getPosition();
          float axePos  = getSortingAxe(position);

          for (int i = 0; i < getNumberOfChilds(); ++i) {
            Widget *child = getChild(i);
            child->setSize(size);
            child->setPosition(position);
            axePos += heightPerChild + DEFAULT_SPACING;
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
  
  ScreenVBox::ScreenVBox(GameLoop *loop, float x, float y, float width, float height) : VBoxWidget(loop)
  {
    setPosition(Vec3(x + DEFAULT_SPACING, y + DEFAULT_SPACING, 1.0f));
    setSize(Vec3(width - DEFAULT_SPACING * 2, height - DEFAULT_SPACING * 2, 1.0f));
  }


  //
  // TextWidget
  // 
  TextWidget::TextWidget(SoFont *font, const String &label)
    : font(font), label(label)
  {
    setPreferedSize(Vec3(SoFont_TextWidth(font, label), SoFont_FontHeight(font), 1.0));
  }

  void TextWidget::draw(SDL_Surface *screen) const
  {
    SoFont_PutString(font, screen, (int)getPosition().x, (int)getPosition().y, (const char*)label, NULL);
  }
};

