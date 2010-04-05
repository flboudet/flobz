#ifndef _IOS_STANDARD_MESSAGE_H
#define _IOS_STANDARD_MESSAGE_H

#include "ios_basemessage.h"

namespace ios_fc {

  class StandardMessage : public BaseMessage {
    public:

      class InvalidMessageException : public Exception {
        public:
          InvalidMessageException() : Exception("Invalid Message") {}
      };

      StandardMessage(int serialID);
      StandardMessage(const Buffer<char> serialized) throw(InvalidMessageException);
      virtual ~StandardMessage();

      bool isReliable() const;
      int  getSerialID() const;
      void send();

      virtual void addInt       (const String &key, int value);
      virtual void addBool      (const String &key, bool value);
      virtual void addFloat     (const String &key, double value);
      virtual void addString    (const String &key, const String &value);
      virtual void addIntArray  (const String &key, const Buffer<int> &value);
      virtual void addCharArray (const String &key, const Buffer<char> &value);

      virtual void addIntProperty   (const String &key, int value);
      virtual void addBoolProperty  (const String &key, bool property);

      virtual VoidBuffer serialize();

    private:
      virtual void sendBuffer(Buffer<char> out) const {}

      AdvancedBuffer<String *> serialized;
      void checkMessage() throw(InvalidMessageException);
  };

}

#endif
