#ifndef _IOS_UDP_MESSAGE_H
#define _IOS_UDP_MESSAGE_H

namespace ios_fc {

  class UDPMessage : public Message {
    public:
      UDPMessage(int serialID/*, const UDPMessageBox &owner*/);
      UDPMessage(const Buffer<char> serialized/*, const UDPMessageBox &owner*/);
      virtual ~UDPMessage();
      
      bool isReliable() const;
      int  getSerialID() const;
      void send() const;

      void addInt       (const String key, int value);
      void addBool      (const String key, bool value);
      void addString    (const String key, const String value);
      void addIntArray  (const String key, const Buffer<int> value);

      const Buffer<char> serialize() const;
      
    private:
      AdvancedBuffer<String *> serialized;
  };

};

#endif
