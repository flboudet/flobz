#ifndef _NOTIFYCENTER_H
#define _NOTIFYCENTER_H

#include "gameui.h"
#include "ios_hash.h"

namespace gameui {
    
    class NotificationResponder {
    public:
        virtual void notificationOccured(String identifier, void * context) {};
    };
    
    class NotifyCenter: HashMap {
    public:
        NotifyCenter() {};
        virtual ~NotifyCenter() {};
        void addListener(String identifier, NotificationResponder * listener);
        void removeListener(String identifier, NotificationResponder * listener);
        void notify(String identifier, void * context);
    };
    
    extern NotifyCenter GlobalNotificationCenter;
    
}


#endif // _NOTIFYCENTER_H
