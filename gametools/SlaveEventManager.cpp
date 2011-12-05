//
//  IPhoneEventManager.mm
//  flobopop
//
//  Created by Florent Boudet on 15/11/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#include "SlaveEventManager.h"

using namespace event_manager;

SlaveGameControlEvent::SlaveGameControlEvent()
{
}

GameControlEvent *SlaveGameControlEvent::clone()
{
    return new SlaveGameControlEvent(*this);
}

GameControlEvent *SlaveEventManager::createGameControlEvent() const
{
    return new SlaveGameControlEvent;
}

bool SlaveEventManager::pollEvent(GameControlEvent &controlEvent)
{
    if (m_eventQueue.empty())
        return false;
    controlEvent = m_eventQueue.front();
    m_eventQueue.pop_front();
    return true;
}

void SlaveEventManager::pushMouseEvent(int x, int y, CursorEventType type)
{
    SlaveGameControlEvent newEvent;
    newEvent.gameEvent = kGameNone;
    newEvent.cursorEvent = type;
    newEvent.keyboardEvent = kKeyboardNone;
    if (type == kGameMouseUp)
        newEvent.isUp = true;
    else newEvent.isUp = false;
    newEvent.isJoystick;
	newEvent.caught;
    newEvent.x = x; newEvent.y = y;
    newEvent.unicodeKeySym = 0;
    m_eventQueue.push_back(newEvent);
}

// Control settings handling
ios_fc::String SlaveEventManager::getControlName(int controlType, bool alternate)
{
}

bool SlaveEventManager::changeControl(int controlType, bool alternate, GameControlEvent &event)
{
    return true;
}

void SlaveEventManager::saveControls()
{
}


