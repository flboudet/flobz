#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

#include "ios_udpmessagebox.h"

namespace ios_fc {

  class UDPMessage : public Message {
    public:

      class InvalidMessageException : public Exception {
        public:
          InvalidMessageException() : Exception("Invalid Message") {}
      };
      
      UDPMessage(int serialID, UDPMessageBox &owner);
      UDPMessage(const Buffer<char> serialized, UDPMessageBox &owner) throw(InvalidMessageException);
      virtual ~UDPMessage();
      
      bool isReliable() const;
      int  getSerialID() const;
      void send() const;

      void addInt       (const String key, int value);
      void addBool      (const String key, bool value);
      void addString    (const String key, const String value);
      void addIntArray  (const String key, const Buffer<int> value);

      void addIntProperty   (const String key, const int value);
      void addBoolProperty  (const String key, const bool property);

      const Buffer<char> serialize() const;
      
    private:
      AdvancedBuffer<String *> serialized;
      UDPMessageBox &owner;
      void checkMessage() throw(InvalidMessageException);
  };

};

#endif
