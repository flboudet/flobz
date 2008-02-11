#include "FramedEditField.h"

namespace gameui {

FramedEditField::FramedEditField(const String &label, Action *action,
				 const FramePicture *framePicture, const FramePicture *focusedPicture)
  : Frame(framePicture), m_editField(label, action)
{
  initFramedEditField(framePicture, focusedPicture);
}

FramedEditField::FramedEditField(const String &defaultText, const String &persistentID,
				 const FramePicture *framePicture, const FramePicture *focusedPicture)
  : Frame(framePicture), m_editField(defaultText, persistentID)
{
  initFramedEditField(framePicture, focusedPicture);
}

void FramedEditField::setValue(String value)
{
  m_editField.setValue(value);
}

void FramedEditField::initFramedEditField(const FramePicture *framePicture, const FramePicture *focusedPicture)
{
  setFocusedPicture(focusedPicture);
  Vec3 preferedSize = m_editField.getPreferedSize();
  preferedSize.x += 25;
  preferedSize.y = 28;
  setPreferedSize(preferedSize);
  add(&m_editField);
}



//
// EditFieldWithLabel
//

EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, Action *action,
				       const FramePicture *framePicture, const FramePicture *focusedPicture)
  : m_editField(defaultValue, action, framePicture, focusedPicture),
    text(label)
{
  add(&text);
  add(&m_editField);
}

EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, String persistentID,
				       const FramePicture *framePicture, const FramePicture *focusedPicture)
  : m_editField(defaultValue, persistentID, framePicture, focusedPicture),
    text(label)
{
  add(&text);
  add(&m_editField);
}

EditFieldWithLabel::~EditFieldWithLabel()
{
}

}

