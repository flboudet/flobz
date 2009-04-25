#include "SwitchedButton.h"
#include "preferences.h"

namespace gameui {

    //
    // SwitchedButton
    //

    SwitchedButton::SwitchedButton(String label, bool defaultValue,
                                   IosSurface *trueSurface, IosSurface *falseSurface, String prefKey, Action * altResponder)
    : stateImage(), imageTrue(trueSurface), imageFalse(falseSurface),
    text(label), key(prefKey), persistant(true), m_altResponder(altResponder)
    {
        if (key == "")
        {
            char tmp[2+sizeof(void *)*2+1];
            snprintf(tmp,sizeof(tmp),"%p",this);
	    tmp[sizeof(tmp)-1] = 0;
            notifKey = String("UI.button.") + tmp;
            stateValue = defaultValue;
            persistant = false;
        } else {
            stateValue = (bool)GetBoolPreference(key, defaultValue);
            persistant = true;
            notifKey = key;
        }
        gameui::GlobalNotificationCenter.addListener(notifKey, this);
        setInnerMargin(0);
        setFocusable(true);
        text.setFocusable(false);
        stateImage.setFocusable(true);
        setPolicy(USE_MAX_SIZE_NO_MARGIN);
        stateImage.setImage(stateValue ? imageTrue : imageFalse);
        stateImage.setOnMouseUpAction(this);
        stateImage.setInvertedFocus(true);
        add(&stateImage);
        text.mdontMove=true;
        add(&text);
        autoSetPreferedSize();
    }

    void SwitchedButton::lostFocus()
    {
        stateImage.lostFocus();
        text.lostFocus();
    }

    void SwitchedButton::giveFocus()
    {
        stateImage.giveFocus();
        text.giveFocus();
    }

    void SwitchedButton::action(Widget *sender, int actionType, GameControlEvent *event)
    {
        if (m_altResponder != NULL) {
            m_altResponder->action(this, actionType, event);
            return;
        }
        if ((actionType == ON_MOUSEUP) || (actionType == ON_MOUSEUP)) {
            bool val = !stateValue;
            gameui::GlobalNotificationCenter.notify(notifKey, &val);
        }
    }

    void SwitchedButton::notificationOccured(String identifier, void * context)
    {
        if (!(identifier == notifKey))
        {
            fprintf(stderr, "Something weird has occured, SwitchedButton registered for notification '%s',\n",(const char *)notifKey);
            fprintf(stderr, "but received one for '%s'... ignoring...\n",(const char *)identifier);
            return;
        }
        stateValue = *(bool*)(context);
        stateImage.setImage(stateValue ? imageTrue : imageFalse);
        if (persistant) SetBoolPreference((const char*)key, (bool)stateValue);
        autoSetPreferedSize();
    }

    void SwitchedButton::setState(bool _state)
    {
        gameui::GlobalNotificationCenter.notify(notifKey, &_state);
    }

    void SwitchedButton::autoSetPreferedSize()
    {
        Vec3 size = text.getPreferedSize();
        Vec3 imgSize = stateImage.getPreferedSize();
        size.x += imgSize.x + 10.0f;
        if (imgSize.y > size.y) size.y = imgSize.y;
        size.y += 2.0f;
        setPreferedSize(size);
    }

    bool SwitchedButton::getState()
    {
        return stateValue;
    }

    SwitchedButton::~SwitchedButton()
    {
        gameui::GlobalNotificationCenter.removeListener(notifKey, this);
    }

}

