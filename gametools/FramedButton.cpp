#include "FramedButton.h"

using namespace event_manager;

namespace gameui {

FramedButton::FramedButton(const String &label, Action *action,
			   const FramePicture *btnPicture, const FramePicture *focusedPicture,
               IosFont *fontActive, IosFont *fontInactive, IosSurface *image)
  : Frame(btnPicture), m_button(label, fontActive, fontInactive)
{
  m_button.mdontMove = true;
  m_button.setAction(ON_ACTION, action);
  setFocusedPicture(focusedPicture);
  Vec3 preferedSize = m_button.getPreferedSize();
  preferedSize.x += 25;
  preferedSize.y = 28;
  setFocusable(true);
  if (image != NULL) {
      m_image.setImage(image);
      m_separator.setPreferedSize(Vec3(2, 0));
      m_box.add(&m_image);
      m_box.add(&m_separator);
      preferedSize.x += 2 + image->w;
  }
  m_box.add(&m_button);
  add(&m_box);
  setPreferedSize(preferedSize);
}

void FramedButton::setValue(String value)
{
  m_button.setValue(value);
}

void FramedButton::eventOccured(GameControlEvent *event)
{
    if ((event->cursorEvent == kGameMouseUp) || (event->cursorEvent == kGameMouseMoved) || (event->cursorEvent == kGameMouseDown)) {
        // If we click inside the frame
        if (isMostlyInside(event->x, event->y))
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

