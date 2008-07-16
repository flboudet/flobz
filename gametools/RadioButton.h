#ifndef _RADIOBUTTON_H
#define _RADIOBUTTON_H

#include "SwitchedButton.h"
#include <vector>

namespace gameui {
    
    class RadioButton : public VBox, Action, NotificationResponder {
    public:
        RadioButton(int defaultValue, IIM_Surface *trueSurface, IIM_Surface *falseSurface, String prefKey);
        virtual ~RadioButton();
        virtual void action(Widget *sender, int actionType, GameControlEvent *event);
        void notificationOccured(String identifier, void * context);
        int getState();
        void setState(int _value);
        void addButton(String _label);
//        void draw(SDL_Surface *screen);
        
    private:
        IIM_Surface * imageTrue;
        IIM_Surface * imageFalse;
        String key, notifKey;
        int stateValue;
        bool persistant;
        std::vector<SwitchedButton *> buttons;
        std::vector<HBox *> containers;
        std::vector<HBox *> spacers;
    };
    
}


#endif // _RADIOBUTTON_H
