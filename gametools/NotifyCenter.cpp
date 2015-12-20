#include "NotifyCenter.h"
#include <stdio.h>
#include <algorithm>

using namespace std;

namespace gameui {

    NotifyCenter GlobalNotificationCenter;

    //
    // NotifyCenter
    //

    void NotifyCenter::addListener(const char *identifier, NotificationResponder * listener)
    {
        std::set<NotificationResponder *> &listeners = m_listeners[identifier];
        // add the listener (even if already present)
        listeners.insert(listener);
    }

    void NotifyCenter::removeListener(const char *identifier, NotificationResponder * listener)
    {
        std::set<NotificationResponder *> &listeners = m_listeners[identifier];
        listeners.erase(listener);
    }

    void NotifyCenter::notify(const char *identifier, void * context)
    {
        // Get the listener list for this notification
        std::set<NotificationResponder *> &listeners = m_listeners[identifier];
        // Iterate for every listener
        std::set<NotificationResponder *>::iterator listenerIterator;
        for (listenerIterator = listeners.begin(); listenerIterator != listeners.end(); listenerIterator++)
        {
            (*listenerIterator)->notificationOccured(identifier, context);
        }
    }

}
