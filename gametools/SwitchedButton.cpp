#include "SwitchedButton.h"
#include "preferences.h"

namespace gameui {
    
    //
    // SwitchedButton
    //
    
    SwitchedButton::SwitchedButton(String label, bool defaultValue,
                                   IIM_Surface *trueSurface, IIM_Surface *falseSurface, String prefKey = "")
    : stateImage(), imageTrue(trueSurface), imageFalse(falseSurface),
    text(label), key(prefKey)
    {
        if (key == "") stateValue = defaultValue;
        else stateValue = (bool)GetBoolPreference(key, defaultValue);
        setInnerMargin(0);
        setFocusable(true);
        text.setFocusable(false);
        stateImage.setFocusable(true);
        setPolicy(USE_MAX_SIZE_NO_MARGIN);
        stateImage.setImage(stateValue ? imageTrue : imageFalse);
        //stateImage.setOnStartAction(this);
        stateImage.setOnMouseUpAction(this);
        stateImage.setInvertedFocus(true);
        add(&stateImage);
        text.mdontMove=true;
        add(&text);
        autoSetPreferedSize();
        gameui::GlobalNotificationCenter.addListener(key, this);
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

    void SwitchedButton::action(Widget *sender, GameUIEnum actionType, GameControlEvent *event)
    {
        if ((actionType == ON_MOUSEUP) || (actionType == ON_MOUSEUP)) {
            stateValue = !stateValue;
            if (key != "") SetBoolPreference((const char*)key, (bool)stateValue);
            gameui::GlobalNotificationCenter.notify(key, &stateValue);
        }
    }
    
    void SwitchedButton::notificationOccured(String identifier, void * context)
    {
        if (!(identifier == key))
        {
            fprintf(stderr, "Something weird has occured, SwitchedButton registered for notification '%s',\n",(const char *)key);
            fprintf(stderr, "but received one for '%s'... ignoring...\n",(const char *)identifier);
            return;
        }
        stateValue = *(bool*)(context);
        stateImage.setImage(stateValue ? imageTrue : imageFalse);
        autoSetPreferedSize();
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
        gameui::GlobalNotificationCenter.removeListener(key, this);
    }
    
}

