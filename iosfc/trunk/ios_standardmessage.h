#ifndef _IOS_STANDARD_MESSAGE_H
#define _IOS_STANDARD_MESSAGE_H

#include "ios_message.h"

namespace ios_fc {

  class StandardMessage : public Message {
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
      void send() const;

      void addInt       (const String key, int value);
      void addBool      (const String key, bool value);
      void addString    (const String key, const String value);
      void addIntArray  (const String key, const Buffer<int> value);
      void addCharArray (const String key, const Buffer<char> value);

      void addIntProperty   (const String key, const int value);
      void addBoolProperty  (const String key, const bool property);

      const Buffer<char> serialize() const;
      
    private:
      virtual void sendBuffer(Buffer<char> out) const {}

      AdvancedBuffer<String *> serialized;
      void checkMessage() throw(InvalidMessageException);
  };

};

#endif
