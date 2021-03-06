#ifndef _SWITCHEDBUTTON_H
#define _SWITCHEDBUTTON_H

#include "gameui.h"
#include "NotifyCenter.h"
#include "PreferencesManager.h"

namespace gameui {

    class SwitchedButton : public HBox, Action, NotificationResponder {
    public:
        SwitchedButton(String label, bool defaultValue,
                       IosSurface *trueSurface, IosSurface *falseSurface,
                       String prefKey, PreferencesManager *prefMgr, Action * altResponder=NULL);
        virtual ~SwitchedButton();
        virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
        void notificationOccured(String identifier, void * context);
        bool getState();
        void setState(bool _state);
        void lostFocus();
        void giveFocus();

    private:
        void autoSetPreferedSize();
        PreferencesManager *m_prefMgr;
        Image stateImage;
        IosSurface * imageTrue;
        IosSurface * imageFalse;
        Button text;
        String key, notifKey;
        bool stateValue, persistant;
        Action * m_altResponder;
    };

}


#endif // _SWITCHEDBUTTON_H
