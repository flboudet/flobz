#ifndef _NOTIFYCENTER_H
#define _NOTIFYCENTER_H

#include <string>
#include <map>
#include <set>

namespace gameui {

    class NotificationResponder {
    public:
        virtual void notificationOccured(const std::string &identifier, void * context) {};
        virtual ~NotificationResponder() {};
    };

    class NotifyCenter {
    public:
        NotifyCenter() {};
        virtual ~NotifyCenter() {};
        void addListener(const std::string &identifier, NotificationResponder * listener);
        void removeListener(const std::string &identifier, NotificationResponder * listener);
        void notify(const std::string &identifier, void * context);
    private:
        typedef std::map<std::string, std::set<NotificationResponder *> > NotifyCenterListeners;
        NotifyCenterListeners m_listeners;
    };

    extern NotifyCenter GlobalNotificationCenter;

}


#endif // _NOTIFYCENTER_H
