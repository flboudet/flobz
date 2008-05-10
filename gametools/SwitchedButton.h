#ifndef _SWITCHEDBUTTON_H
#define _SWITCHEDBUTTON_H

#include "gameui.h"
#include "NotifyCenter.h"

namespace gameui {
    
    class SwitchedButton : public HBox, Action, NotificationResponder {
    public:
        SwitchedButton(String label, bool defaultValue,
                       IIM_Surface *trueSurface, IIM_Surface *falseSurface, String prefKey);
        virtual ~SwitchedButton();
        virtual void action(Widget *sender, int actionType, GameControlEvent *event);
        void notificationOccured(String identifier, void * context);
        bool getState();
        void lostFocus();
        void giveFocus();

    private:
        void autoSetPreferedSize();
        Image stateImage;
        IIM_Surface * imageTrue;
        IIM_Surface * imageFalse;
        Button text;
        String key;
        bool stateValue;
    };
    
}


#endif // _SWITCHEDBUTTON_H
