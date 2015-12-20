#ifndef _NOTIFYCENTER_H
#define _NOTIFYCENTER_H

#include <string>
#include <map>
#include <set>

namespace gameui {

    class NotificationResponder {
    public:
        virtual void notificationOccured(const char *identifier, void * context) {};
        virtual ~NotificationResponder() {};
    };

    class NotifyCenter {
    public:
        NotifyCenter() {};
        virtual ~NotifyCenter() {};
        void addListener(const char *identifier, NotificationResponder * listener);
        void removeListener(const char *identifier, NotificationResponder * listener);
        void notify(const char *identifier, void * context);
    private:
        typedef std::map<std::string, std::set<NotificationResponder *> > NotifyCenterListeners;
        NotifyCenterListeners m_listeners;
    };

    extern NotifyCenter GlobalNotificationCenter;

}


#endif // _NOTIFYCENTER_H
