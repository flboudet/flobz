/*
 *  ListWidget.cpp
 *  flobopuyo
 *
 *  Created by Florent Boudet on 17/12/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ListView.h"

namespace gameui {

void ScrollWidget::eventOccured(GameControlEvent *event)
{
    if (event->cursorEvent == GameControlEvent::kGameMouseClicked) {
        Vec3 widPosition = getPosition();
        Vec3 widSize = getSize();
        if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
            && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y)) {
            float firstVisible = m_siProvider.getFirstVisible();
            float lastVisible = m_siProvider.getLastVisible();
            float numItems = m_siProvider.getFullSize();
            
            int firstVisibleOffset = widSize.y * firstVisible / numItems;
            int lastVisibleOffset = widSize.y * lastVisible / numItems;
            
            // Click above the cursor
            if (event->y < widPosition.y + firstVisibleOffset) {
                m_siProvider.setFirstVisible(firstVisible / 2.);
            }
            else if (event->y > widPosition.y + lastVisibleOffset) {
                m_siProvider.setFirstVisible(firstVisible + (numItems - lastVisible) / 2.);
            }
        }
    }
}

void ScrollWidget::draw(SDL_Surface *screen)
{
    float firstVisible = m_siProvider.getFirstVisible();
    float lastVisible = m_siProvider.getLastVisible();
    float numItems = m_siProvider.getFullSize();
    
    int firstVisibleOffset = getSize().y * firstVisible / numItems;
    int lastVisibleOffset = getSize().y * lastVisible / numItems;
    
    SDL_Rect dstrect;
    
    dstrect.x = getPosition().x;
    dstrect.y = getPosition().y;
    dstrect.h = getSize().y;
    dstrect.w = getSize().x;
    SDL_FillRect(screen, &dstrect, 0xFF550000);
    
    dstrect.y += firstVisibleOffset;
    dstrect.h = lastVisibleOffset - firstVisibleOffset;
    SDL_FillRect(screen, &dstrect, 0xFFAAAAAA);
}



//
// ListView
//

ListView::ListView(int size, IIM_Surface *downArrow, GameLoop *loop) : CycledComponent(0.1), HBox(loop), size(size), firstVisible(0), used(0), scrollWidget(*this), m_clickedButton(NULL), m_cyclesBeforeContinuousScroll(0)
{
    suspendLayout();
    scrollerBox.suspendLayout();
    listBox.suspendLayout();
    
    setPolicy(USE_MIN_SIZE);
    upButton.setImage(downArrow);
    downButton.setImage(downArrow);
    upButton.setOnStartAction(this);
    upButton.setOnMouseUpAction(this);
    downButton.setOnStartAction(this);
    downButton.setOnMouseUpAction(this);
    scrollerBox.setPreferedSize(Vec3(16, (size+2) + SoFont_FontHeight(GameUIDefaults::FONT)*size));
    //listBox.setPreferedSize(Vec3(150,(size+2) + SoFont_FontHeight(GameUIDefaults::FONT)*size, 1));
    //listBox.setPolicy(USE_MIN_SIZE);
    scrollerBox.setPolicy(USE_MIN_SIZE);
    scrollerBox.add(&upButton);
    scrollerBox.add(&scrollWidget);
    scrollerBox.add(&downButton);
    
    for (int i=0; i<size; ++i) {
        buttons.push_back(new Button(""));
        listBox.add(buttons.back());
    }
    
    HBox::add(&listBox);
    HBox::add(&scrollerBox);

    scrollerBox.resumeLayout();
    listBox.resumeLayout();
    resumeLayout();
}

void ListView::addEntry(const ListViewEntry &entry)
{
    entries.push_back(entry);
    resyncLabels();
}

void ListView::draw(SDL_Surface *screen)
{
    SDL_Rect dstrect;
    
    dstrect.x = getPosition().x;
    dstrect.y = getPosition().y;
    dstrect.h = getSize().y;
    dstrect.w = getSize().x;
    SDL_FillRect(screen, &dstrect, 0x55555555);
    
    dstrect.x = listBox.getPosition().x;
    dstrect.y = listBox.getPosition().y;
    dstrect.h = listBox.getSize().y;
    dstrect.w = listBox.getSize().x;
    SDL_FillRect(screen, &dstrect, 0xFFFFFFFF);
    
    dstrect.x = scrollerBox.getPosition().x;
    dstrect.y = scrollerBox.getPosition().y;
    dstrect.h = scrollerBox.getSize().y;
    dstrect.w = scrollerBox.getSize().x;
    SDL_FillRect(screen, &dstrect, 0xFFFFFF00);
    
    HBox::draw(screen);
}

void ListView::eventOccured(GameControlEvent *event)
{
    /*int direction = 0;
    if (listBox.isPrevEvent(event)) direction = -1;
    else if (listBox.isNextEvent(event)) direction = 1;
    if (direction != 0) {
        if (listBox.getActiveWidget() + direction >= listBox.getNumberOfChilds()) {}
        else if (listBox.getActiveWidget() + direction < 0) {}
    }
    else */HBox::eventOccured(event);
}

void ListView::action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
{
    switch (actionType) {
        case ON_START:
            m_clickedButton = sender;
            handleButtons();
            break;
        case ON_MOUSEUP:
            m_clickedButton = NULL;
            break;
    }
}

void ListView::handleButtons()
{
    if (m_clickedButton == &downButton) {
        if (firstVisible + size < entries.size()) {
            firstVisible++;
            resyncLabels();
        }
    }
    else if (m_clickedButton == &upButton) {
        if (firstVisible > 0) {
            firstVisible--;
            resyncLabels();
        }
    }
}

void ListView::cycle()
{
    if (m_clickedButton != NULL) {
        if (m_cyclesBeforeContinuousScroll++ > 3)
            handleButtons();
    }
    else
        m_cyclesBeforeContinuousScroll = 0;
}

void ListView::resyncLabels()
{
    int lastVisible = size < entries.size() ? firstVisible + size : firstVisible + entries.size();
    for (int i = firstVisible ; i < lastVisible ; i++) {
        buttons[i-firstVisible]->setValue(entries[i].m_text);
        buttons[i-firstVisible]->setAction(ON_START, entries[i].m_action);
    }
}

void ListView::setFirstVisible(float firstVisible)
{
    if ((firstVisible >= 0) && (firstVisible + size < entries.size())) {
            this->firstVisible = firstVisible;
            resyncLabels();
    }
}

}


