//
//  IPhoneEventManager.h
//  flobopop
//
//  Created by Florent Boudet on 15/11/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifndef _SLAVEEVENTMANAGER_H
#define _SLAVEEVENTMANAGER_H

#include <deque>
#include "GameControls.h"

class SlaveGameControlEvent : public event_manager::GameControlEvent
{
public:
    SlaveGameControlEvent();
    virtual event_manager::GameControlEvent *clone();
};

class SlaveEventManager : public event_manager::EventManager
{
public:
    virtual event_manager::GameControlEvent *createGameControlEvent() const;
    virtual bool pollEvent(event_manager::GameControlEvent &controlEvent);
    virtual void pushMouseEvent(int x, int y, event_manager::CursorEventType type);
    // Control settings handling
    virtual ios_fc::String getControlName(int controlType, bool alternate);
    virtual bool   changeControl(int controlType, bool alternate, event_manager::GameControlEvent &event);
    virtual void   saveControls();
    virtual void setEnableJoyMouseEmulation(bool enabled) {}
private:
    std::deque<SlaveGameControlEvent> m_eventQueue;
};

#endif // _SLAVEEVENTMANAGER_H

