#ifndef _FRAMEDBUTTON_H
#define _FRAMEDBUTTON_H

#include "gameui.h"
#include "Frame.h"

namespace gameui {


class FramedButton : public Frame {
public:
  FramedButton(const String &label, Action *action,
	           const FramePicture *btnPicture, const FramePicture *focusedPicture,
               IosFont *fontActive = NULL, IosFont *fontInactive = NULL,
               IosSurface *image = NULL);
  virtual ~FramedButton() {}
  virtual void setValue(String value);
  Button *getButton() { return &m_button; }
  void eventOccured(event_manager::GameControlEvent *event);
private:
  Button m_button;
  Image  m_image;
  Separator m_separator;
  HBox   m_box;
};

}

#endif // _FRAMEDBUTTON_H

