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

ScrollWidget::ScrollWidget(ScrollInfoProvider &siProvider)
    : m_bgSurface(NULL), m_siProvider(siProvider), m_grabbing(false)
{
    Widget::setFocusable(true);
    setReceiveUpEvents(true);
}

ScrollWidget::~ScrollWidget()
{
    if (m_bgSurface != NULL)
        IIM_Free(m_bgSurface);
}

void ScrollWidget::eventOccured(GameControlEvent *event)
{
    if (isDirectionEvent(event))
            lostFocus();
    if (event->cursorEvent == GameControlEvent::kGameMouseClicked) {
        if (event->isUp) {
            if (m_grabbing) {
                getParentScreen()->ungrabEventsOnWidget(this);
                m_grabbing = false;
            }
            return;
        }
        Vec3 widPosition = getPosition();
        Vec3 widSize = getSize();
        if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
            && (widPosition.x + widSize.x >= widPosition.x) && (widPosition.y + widSize.y >= event->y)) {
            float firstVisible = m_siProvider.getFirstVisible();
            float lastVisible = m_siProvider.getLastVisible();
            float numItems = m_siProvider.getFullSize();
            
            int firstVisibleOffset = (int)(widSize.y * firstVisible / numItems);
            int lastVisibleOffset = (int)(widSize.y * lastVisible / numItems);
            
            // Click above the cursor
            if (event->y < widPosition.y + firstVisibleOffset) {
                m_siProvider.setFirstVisible(firstVisible / 2.);
            }
            else if (event->y > widPosition.y + lastVisibleOffset) {
                m_siProvider.setFirstVisible(firstVisible + (numItems - lastVisible) / 2.);
            }
            else {
                m_grabbing = true;
                m_grabOffset = (int)(event->y - widPosition.y - firstVisibleOffset);
                getParentScreen()->grabEventsOnWidget(this);
            }
        }
    }
    else if (event->cursorEvent == GameControlEvent::kGameMouseMoved) {
        if (m_grabbing) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            float firstVisible = m_siProvider.getFirstVisible();
            float lastVisible = m_siProvider.getLastVisible();
            float numItems = m_siProvider.getFullSize();
            int eventY = event->y - m_grabOffset;
            if (eventY < widPosition.y)
                m_siProvider.setFirstVisible(0.);
            else if (event->y > widPosition.y + widSize.y)
                m_siProvider.setFirstVisible(numItems + firstVisible - lastVisible);
            else {
                float mouseOffset = eventY - widPosition.y;
                m_siProvider.setFirstVisible(numItems * (mouseOffset / widSize.y));
            }
        }
    }
}

void ScrollWidget::draw(SDL_Surface *screen)
{
    Vec3 bsize = getSize();
    SDL_Rect srcrect, dstrect;
    
    srcrect.x = 0;
    srcrect.y = 0;
    srcrect.h = bsize.y;
    srcrect.w = bsize.x;
    
    dstrect.x = getPosition().x;
    dstrect.y = getPosition().y;
    dstrect.h = bsize.y;
    dstrect.w = bsize.x;
    
    if ((m_bgSurface == NULL) || ((int)(bsize.x) != m_bgSurface->w) || ((int)(bsize.y) != m_bgSurface->h)) {
        if (m_bgSurface != NULL)
            IIM_Free(m_bgSurface);
        m_bgSurface = iim_surface_create_rgba((int)(bsize.x), (int)(bsize.y));
        SDL_FillRect(m_bgSurface->surf, &srcrect, (m_bgSurface->surf->format->Rmask & 0x80808080) | (m_bgSurface->surf->format->Amask & 0x80808080));
    }
    
    float firstVisible = m_siProvider.getFirstVisible();
    float lastVisible = m_siProvider.getLastVisible();
    float numItems = m_siProvider.getFullSize();
    
    int firstVisibleOffset = bsize.y * firstVisible / numItems;
    int lastVisibleOffset = lastVisible > numItems ? bsize.y : bsize.y * lastVisible / numItems;
    
    // Drawing the background
    IIM_BlitSurface(m_bgSurface, &srcrect, screen, &dstrect);
    
    // Drawing the thumb
    dstrect.y += firstVisibleOffset;
    dstrect.h = lastVisibleOffset - firstVisibleOffset;
    SDL_FillRect(screen, &dstrect, 0xAAAAAAAA);
}

//
// ListViewEntry
//

void ListViewEntry::setText(String text)
{
    m_text = text;
    if (m_owner != NULL)
        m_owner->resyncLabels();
}

//
// ListView
//

ListView::ListView(int size, IIM_Surface *upArrow, IIM_Surface *downArrow,
		   const FramePicture *listViewFramePicture, GameLoop *loop)
 : CycledComponent(0.1), HBox(loop), size(size), firstVisible(0), used(0),
   scrollWidget(*this), listBox(listViewFramePicture, loop),
   m_clickedButton(NULL), m_cyclesBeforeContinuousScroll(0)
{
    suspendLayout();
    scrollerBox.suspendLayout();
    listBox.suspendLayout();
    
    setPolicy(USE_MIN_SIZE);
    listBox.setPolicy(USE_MAX_SIZE);
    upButton.setImage(upArrow);
    downButton.setImage(downArrow);
    upButton.setOnStartAction(this);
    upButton.setOnMouseUpAction(this);
    downButton.setOnStartAction(this);
    downButton.setOnMouseUpAction(this);
    scrollerBox.setPreferedSize(Vec3(16., 0.));
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

ListView::~ListView()
{
}

void ListView::addEntry(ListViewEntry *entry)
{
    entries.push_back(entry);
    entry->m_owner = this;
    resyncLabels();
}

void ListView::removeEntry(ListViewEntry *entry)
{
    for (std::vector<ListViewEntry *>::iterator iter = entries.begin() ;
         iter < entries.end() ; iter++) {
         if (*iter == entry) {
            entry->m_owner = NULL;
            entries.erase(iter);
            break;
         }
    }
    resyncLabels();
}

void ListView::eventOccured(GameControlEvent *event)
{
    HBox::eventOccured(event);
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
        default:
            break;
    }
}

void ListView::handleButtons()
{
    if (m_clickedButton == &downButton) {
        if (firstVisible + size < (int)entries.size()) {
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
    if (! m_clickedButton->haveFocus())
        m_clickedButton = NULL;
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
    int lastVisible = size < (int)entries.size() ? firstVisible + size : firstVisible + entries.size();
    for (int i = firstVisible ; i < lastVisible ; i++) {
        buttons[i-firstVisible]->setValue(entries[i]->m_text);
        buttons[i-firstVisible]->setAction(ON_START, entries[i]->m_action);
        buttons[i-firstVisible]->setFocusable(true);
    }
    for (int i = lastVisible ; i < firstVisible + size ; i++) {
        buttons[i-firstVisible]->setValue("");
        buttons[i-firstVisible]->setAction(ON_START, NULL);
        buttons[i-firstVisible]->setFocusable(false);
    }
}

void ListView::setFirstVisible(float firstVisible)
{
    if (((int)firstVisible >= 0) && ((int)firstVisible + size <= (int)(entries.size()))) {
            this->firstVisible = (int)firstVisible;
            resyncLabels();
    }
}

}


