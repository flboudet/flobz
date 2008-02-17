#ifndef _FRAMEDEDITFIELD_H
#define _FRAMEDEDITFIELD_H

#include "gameui.h"
#include "Frame.h"

namespace gameui {

class FramedEditField : public Frame {
public:
  FramedEditField(const String &label, Action *action,
		  const FramePicture *framePicture, const FramePicture *focusedPicture);
  FramedEditField(const String &defaultText, const String &persistentID,
		  const FramePicture *framePicture, const FramePicture *focusedPicture);
  virtual ~FramedEditField() {}
  virtual void setValue(String value);
  EditField & getEditField() { return m_editField; }
  const EditField & getEditField() const { return m_editField; }
private:
  void initFramedEditField(const FramePicture *framePicture, const FramePicture *focusedPicture);
  EditField m_editField;
};

class EditFieldWithLabel : public HBox {
public:
  EditFieldWithLabel(String label, String defaultValue, Action *action,
		     const FramePicture *framePicture, const FramePicture *focusedPicture,
		     int width = 0);
  EditFieldWithLabel(String label, String defaultValue, String persistentID,
		     const FramePicture *framePicture, const FramePicture *focusedPicture,
		     int width = 0);
  virtual ~EditFieldWithLabel();
  EditField &getEditField() { return m_editField.getEditField(); }
  const EditField &getEditField() const { return m_editField.getEditField(); }
private:
  FramedEditField m_editField;
  Text text;
};

}

#endif // _FRAMEDEDITFIELD_H

