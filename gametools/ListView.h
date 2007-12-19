/*
 *  ListWidget.h
 *  flobopuyo
 *
 *  Created by Florent Boudet on 17/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include "gameui.h"

namespace gameui {

class ScrollInfoProvider {
public:
    virtual float getFirstVisible() const = 0;
    virtual float getLastVisible() const = 0;
    virtual float getFullSize() const = 0;
    virtual void setFirstVisible(float firstVisible) = 0;
};

class ScrollWidget : public Widget {
public:
    ScrollWidget(ScrollInfoProvider &siProvider) : m_siProvider(siProvider) {Widget::setFocusable(true);}
    virtual void eventOccured(GameControlEvent *event);
protected:
    virtual void draw(SDL_Surface *screen);
private:
    ScrollInfoProvider &m_siProvider;
};

class ListViewEntry {
public:
    ListViewEntry(String text, Action *action = NULL) : m_text(text), m_action(action) {}
private:
    String m_text;
    Action *m_action;
friend class ListView;
};

class ListView : public CycledComponent, public HBox, public Action, public ScrollInfoProvider {
public:
    ListView(int size, IIM_Surface *downArrow, GameLoop *loop = NULL);
    void addEntry(const ListViewEntry &entry);
    virtual void eventOccured(GameControlEvent *event);
    virtual IdleComponent *getIdleComponent() { return this; }
    // Action
    virtual void action(Widget *sender, GameUIEnum actionType, GameControlEvent *event);
    // CycledComponent
    virtual void cycle();
    // ScrollInfoProvider
    virtual float getFirstVisible() const { return firstVisible; }
    virtual float getLastVisible() const { return firstVisible + size; }
    virtual float getFullSize() const { return entries.size(); }
    virtual void setFirstVisible(float firstVisible);
protected:
    virtual void draw(SDL_Surface *screen);
private:
    void resyncLabels();
    void handleButtons();
    
    int size, firstVisible;
    int used;
    Image downButton, upButton;
    ScrollWidget scrollWidget;
    VBox scrollerBox;
    VBox listBox;
    std::vector<ListViewEntry> entries;
    std::vector<Button *> buttons;
    Widget *m_clickedButton;
    int m_cyclesBeforeContinuousScroll;
};

}

#endif // LISTVIEW_H

