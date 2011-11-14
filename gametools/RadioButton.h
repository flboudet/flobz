#ifndef _RADIOBUTTON_H
#define _RADIOBUTTON_H

#include <vector>
#include "SwitchedButton.h"
#include "PreferencesManager.h"

namespace gameui {

    class RadioButton : public VBox, Action, NotificationResponder {
    public:
        RadioButton(int defaultValue, IosSurface *trueSurface, IosSurface *falseSurface, String prefKey, PreferencesManager *prefMgr);
        virtual ~RadioButton();
        virtual void action(Widget *sender, int actionType, event_manager::GameControlEvent *event);
        void notificationOccured(String identifier, void * context);
        int getState();
        void setState(int _value);
        void addButton(String _label);
//        void draw(SDL_Surface *screen);

    private:
        PreferencesManager *m_prefMgr;
        IosSurface * imageTrue;
        IosSurface * imageFalse;
        String key, notifKey;
        int stateValue;
        bool persistant;
        std::vector<SwitchedButton *> buttons;
        std::vector<HBox *> containers;
        std::vector<HBox *> spacers;
    };

}


#endif // _RADIOBUTTON_H
