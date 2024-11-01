#ifndef _FRAMEDEDITFIELD_H
#define _FRAMEDEDITFIELD_H

#include "gameui.h"
#include "Frame.h"

namespace gameui {

class FramedEditField : public Frame {
public:
  FramedEditField(const std::string &label, Action *action,
		  const FramePicture *framePicture, const FramePicture *focusedPicture);
  FramedEditField(const std::string &defaultText, const std::string &persistentID, PreferencesManager *prefMgr,
		  const FramePicture *framePicture, const FramePicture *focusedPicture);
  virtual ~FramedEditField() {}
  virtual void setValue(const std::string &value);
  EditField & getEditField() { return m_editField; }
  const EditField & getEditField() const { return m_editField; }
private:
  void initFramedEditField(const FramePicture *framePicture, const FramePicture *focusedPicture);
  EditField m_editField;
  void eventOccured(event_manager::GameControlEvent *event);
};

class EditFieldWithLabel : public HBox {
public:
  EditFieldWithLabel(const std::string &label, const std::string &defaultValue, Action *action,
		     const FramePicture *framePicture, const FramePicture *focusedPicture,
		     int width = 0);
  EditFieldWithLabel(const std::string &label, const std::string &defaultValue, const std::string &persistentID, PreferencesManager *prefMgr,
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

