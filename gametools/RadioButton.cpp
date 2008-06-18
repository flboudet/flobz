#include "RadioButton.h"
#include "preferences.h"

namespace gameui {
    
    //
    // RadioButton
    //
    
    RadioButton::RadioButton(int defaultValue, IIM_Surface *trueSurface, IIM_Surface *falseSurface, String prefKey)
    : imageTrue(trueSurface), imageFalse(falseSurface),
    key(prefKey), persistant(true)
    {
        if (key == "") 
        {
            char tmp[sizeof(void *)+1];
            sprintf(tmp,"%p",this);
            notifKey = String("UI.radio.") + tmp;
            stateValue = defaultValue;
            persistant = false;
        } else {
            stateValue = (int)GetIntPreference(key, defaultValue);
            persistant = true;
            notifKey = key;
        }        
        gameui::GlobalNotificationCenter.addListener(notifKey, this);
        setInnerMargin(0);

        setPolicy(USE_MIN_SIZE);
    }
    
    void RadioButton::addButton(String _label)
    {
        bool value = (stateValue == (int)buttons.size()+1);
        buttons.push_back(new SwitchedButton(_label, value, imageTrue, imageFalse, String(""), this));
        add(buttons[buttons.size()-1]);
    }
    
    void RadioButton::action(Widget *sender, int actionType, GameControlEvent *event)
    {
        if ((actionType == ON_MOUSEUP) || (actionType == ON_MOUSEUP)) {
            vector<SwitchedButton *>::iterator myIntVectorIterator;
            int pos = 1;
            for(myIntVectorIterator = buttons.begin(); 
                myIntVectorIterator != buttons.end();
                myIntVectorIterator++)
            {
                if ((*myIntVectorIterator) == (SwitchedButton *)sender) {
                    gameui::GlobalNotificationCenter.notify(notifKey, &pos);
                    break;
                }
                pos++;
            }
        }
    }
    
    void RadioButton::setState(int _value) {
         gameui::GlobalNotificationCenter.notify(notifKey, &_value);
    }

    void RadioButton::notificationOccured(String identifier, void * context)
    {
        if (!(identifier == notifKey))
        {
            fprintf(stderr, "Something weird has occured, RadioButton registered for notification '%s',\n",(const char *)notifKey);
            fprintf(stderr, "but received one for '%s'... ignoring...\n",(const char *)identifier);
            return;
        }
        
        if ((*(int*)(context)<=(int)buttons.size()) && (*(int*)(context)>0)) {
            stateValue = *(int*)(context);
            for (int i = buttons.size(); i>0; i--) {
                buttons[i-1]->setState(stateValue==i);
            }
        } else {
            fprintf(stderr, "RadioButton received out of bound notification '%s' item %d for size %d,\n",(const char *)notifKey, stateValue, (int)buttons.size());
        }
        if (persistant) SetIntPreference((const char*)key, (int)stateValue);
    }

    int RadioButton::getState()
    {
        return stateValue;
    }
    
    RadioButton::~RadioButton()
    {
        gameui::GlobalNotificationCenter.removeListener(notifKey, this);
    }
    
}

