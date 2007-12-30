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

class ListView;

class ScrollInfoProvider {
public:
    virtual float getFirstVisible() const = 0;
    virtual float getLastVisible() const = 0;
    virtual float getFullSize() const = 0;
    virtual void setFirstVisible(float firstVisible) = 0;
};

class ScrollWidget : public Widget {
public:
    ScrollWidget(ScrollInfoProvider &siProvider) : m_siProvider(siProvider), m_grabbing(false) {Widget::setFocusable(true); setReceiveUpEvents(true);}
    virtual void eventOccured(GameControlEvent *event);
protected:
    virtual void draw(SDL_Surface *screen);
private:
    ScrollInfoProvider &m_siProvider;
    bool m_grabbing;
    int m_grabOffset;
};

class ListViewEntry {
public:
    ListViewEntry(String text, Action *action = NULL) : m_text(text), m_action(action), m_owner(NULL) {}
    String getText() const { return m_text; }
    void setText(String text);
private:
    String m_text;
    Action *m_action;
    ListView *m_owner;
friend class ListView;
};

class ListView : public CycledComponent, public HBox, public Action, public ScrollInfoProvider {
public:
    ListView(int size, IIM_Surface *downArrow, GameLoop *loop = NULL);
    void addEntry(ListViewEntry *entry);
    void removeEntry(ListViewEntry *entry);
    ListViewEntry *getEntryAt(int index) const { return entries[index]; }
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
    std::vector<ListViewEntry *> entries;
    std::vector<Button *> buttons;
    Widget *m_clickedButton;
    int m_cyclesBeforeContinuousScroll;
friend class ListViewEntry;
};

}

#endif // LISTVIEW_H

