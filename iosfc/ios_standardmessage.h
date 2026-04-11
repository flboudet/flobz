#ifndef _IOS_STANDARD_MESSAGE_H
#define _IOS_STANDARD_MESSAGE_H

#include "ios_basemessage.h"

namespace ios_fc {

  class StandardMessage : public BaseMessage {
    public:
      StandardMessage(int serialID);
      StandardMessage(const Buffer<char> serialized);
      virtual ~StandardMessage();

      virtual void addInt       (const std::string &key, int value);
      virtual void addBool      (const std::string &key, bool value);
      virtual void addFloat     (const std::string &key, double value);
      virtual void addString    (const std::string &key, const std::string &value);
      virtual void addIntArray  (const std::string &key, const Buffer<int> &value);
      virtual void addCharArray (const std::string &key, const Buffer<char> &value);

      virtual void addIntProperty   (const std::string &key, int value);
      virtual void addBoolProperty  (const std::string &key, bool property);

      virtual VoidBuffer serialize();

    private:
      
      AdvancedBuffer<std::string *> serialized;
      void checkMessage();
  };

}

#endif
