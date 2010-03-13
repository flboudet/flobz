#include "FramedButton.h"

using namespace event_manager;

namespace gameui {

FramedButton::FramedButton(const String &label, Action *action,
			   const FramePicture *btnPicture, const FramePicture *focusedPicture)
  : Frame(btnPicture), m_button(label, action)
{
  m_button.mdontMove = true;
  setFocusedPicture(focusedPicture);
  Vec3 preferedSize = m_button.getPreferedSize();
  preferedSize.x += 25;
  preferedSize.y = 28;
  setPreferedSize(preferedSize);
  setFocusable(true);
  //m_button.setFocusable(false);
  add(&m_button);
}

void FramedButton::setValue(String value)
{
  m_button.setValue(value);
}

void FramedButton::eventOccured(GameControlEvent *event)
{
    if ((event->cursorEvent == kGameMouseUp) || (event->cursorEvent == kGameMouseDown)) {
        Vec3 widPosition = getPosition();
        Vec3 widSize = getSize();
        // If we click inside the frame
        if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
            && (widPosition.x + widSize.x >= event->x) && (widPosition.y + widSize.y >= event->y))
        {
            // Reposition the click so it's inside the widget
            Vec3 buttonPosition = m_button.getPosition();
            Vec3 buttonSize = m_button.getSize();
            if (event->x < buttonPosition.x) event->x = (int)ceil(buttonPosition.x);
            if (event->x > buttonPosition.x + buttonSize.x) event->x = (int)floor(buttonPosition.x + buttonSize.x);
            if (event->y < buttonPosition.y) event->y = (int)ceil(buttonPosition.y);
            if (event->y > buttonPosition.y + buttonSize.y) event->y = (int)floor(buttonPosition.y + buttonSize.y);
        }
    }

    // Push the event to the widget
    Frame::eventOccured(event);
}

}

