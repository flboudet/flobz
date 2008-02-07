#include "FramedButton.h"

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
  add(&m_button);
}

void FramedButton::setValue(String value)
{
  m_button.setValue(value);
}

}

