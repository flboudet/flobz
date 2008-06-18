#ifndef _SWITCHEDBUTTON_H
#define _SWITCHEDBUTTON_H

#include "gameui.h"
#include "NotifyCenter.h"

namespace gameui {
    
    class SwitchedButton : public HBox, Action, NotificationResponder {
    public:
        SwitchedButton(String label, bool defaultValue,
                       IIM_Surface *trueSurface, IIM_Surface *falseSurface, String prefKey, Action * altResponder=NULL);
        virtual ~SwitchedButton();
        virtual void action(Widget *sender, int actionType, GameControlEvent *event);
        void notificationOccured(String identifier, void * context);
        bool getState();
        void setState(bool _state);
        void lostFocus();
        void giveFocus();

    private:
        void autoSetPreferedSize();
        Image stateImage;
        IIM_Surface * imageTrue;
        IIM_Surface * imageFalse;
        Button text;
        String key, notifKey;
        bool stateValue, persistant;
        Action * m_altResponder;
    };
    
}


#endif // _SWITCHEDBUTTON_H
