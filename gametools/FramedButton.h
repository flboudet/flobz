#ifndef _FRAMEDBUTTON_H
#define _FRAMEDBUTTON_H

#include "gameui.h"
#include "Frame.h"

namespace gameui {

class FramedButton : public Frame {
public:
  FramedButton(const String &label, Action *action,
	       const FramePicture *btnPicture, const FramePicture *focusedPicture);
  virtual ~FramedButton() {}
  virtual void setValue(String value);
private:
  Button m_button;
};

}

#endif // _FRAMEDBUTTON_H

