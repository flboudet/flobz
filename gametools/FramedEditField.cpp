#include "FramedEditField.h"

using namespace event_manager;

namespace gameui {

FramedEditField::FramedEditField(const String &label, Action *action,
				 const FramePicture *framePicture, const FramePicture *focusedPicture)
  : Frame(framePicture), m_editField(label, action)
{
  initFramedEditField(framePicture, focusedPicture);
}

FramedEditField::FramedEditField(const String &defaultText, const String &persistentID, PreferencesManager *prefMgr,
				 const FramePicture *framePicture, const FramePicture *focusedPicture)
  : Frame(framePicture), m_editField(defaultText, persistentID, prefMgr)
{
  initFramedEditField(framePicture, focusedPicture);
}

void FramedEditField::setValue(String value)
{
  m_editField.setValue(value);
}

void FramedEditField::initFramedEditField(const FramePicture *framePicture, const FramePicture *focusedPicture)
{
  innerMargin = framePicture->getMaxMargin();
  setFocusedPicture(focusedPicture);
  Vec3 preferedSize = m_editField.getPreferedSize();
  preferedSize.x = 0.0f;
  preferedSize.y += 2.0f*innerMargin;
  setPreferedSize(preferedSize);
  add(&m_editField);
}

    void FramedEditField::eventOccured(GameControlEvent *event)
    {
        if (event->cursorEvent == kGameMouseMoved
            || event->cursorEvent == kGameMouseDown
			|| event->cursorEvent == kGameMouseUp) {
            Vec3 widPosition = getPosition();
            Vec3 widSize = getSize();
            // If we click inside the frame
            if ((widPosition.x <= event->x) && (widPosition.y <= event->y)
                && (widPosition.x + widSize.x >= event->x) && (widPosition.y + widSize.y >= event->y))
            {
                // Reposition the click so it's inside the widget
                Vec3 buttonPosition = m_editField.getPosition();
                Vec3 buttonSize = m_editField.getSize();
                if (event->x < buttonPosition.x) event->x = (int)ceil(buttonPosition.x);
                if (event->x > buttonPosition.x + buttonSize.x) event->x = (int)floor(buttonPosition.x + buttonSize.x);
                if (event->y < buttonPosition.y) event->y = (int)ceil(buttonPosition.y);
                if (event->y > buttonPosition.y + buttonSize.y) event->y = (int)floor(buttonPosition.y + buttonSize.y);
            }
        }

        // Push the event to the widget
        Frame::eventOccured(event);
    }


//
// EditFieldWithLabel
//

EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, Action *action,
				       const FramePicture *framePicture, const FramePicture *focusedPicture,
				       int width)
  : m_editField(defaultValue, action, framePicture, focusedPicture),
    text(label)
{
    innerMargin = 0.0f;
  setPolicy(USE_MIN_SIZE);
  setPreferedSize(Vec3(0.0f, m_editField.getPreferedSize().y, 0.0f));
  add(&text);
  add(&m_editField);
}

EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, String persistentID, PreferencesManager *prefMgr,
				       const FramePicture *framePicture, const FramePicture *focusedPicture,
				       int width)
  : m_editField(defaultValue, persistentID, prefMgr, framePicture, focusedPicture),
    text(label)
{
    innerMargin = 0.0f;
  setPolicy(USE_MIN_SIZE);
  setPreferedSize(Vec3(0.0f, m_editField.getPreferedSize().y, 0.0f));
  m_editField.setPreferedSize(Vec3(width, m_editField.getPreferedSize().y, 0.0f));
  add(&text);
  add(&m_editField);
}


EditFieldWithLabel::~EditFieldWithLabel()
{
}

}

