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
  innerMargin = framePicture->getMaxMargin();
  setFocusedPicture(focusedPicture);
  Vec3 preferedSize = m_editField.getPreferedSize();
  preferedSize.x = 0.0f;
  preferedSize.y += 2.0f*innerMargin;
  setPreferedSize(preferedSize);
  add(&m_editField);
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

EditFieldWithLabel::EditFieldWithLabel(String label, String defaultValue, String persistentID,
				       const FramePicture *framePicture, const FramePicture *focusedPicture,
				       int width)
  : m_editField(defaultValue, persistentID, framePicture, focusedPicture),
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

