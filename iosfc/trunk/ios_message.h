#ifndef _IOS_MESSAGE_H
#define _IOS_MESSAGE_H

namespace ios_fc {


class Message
{
  public:

    class DataException : public Exception {
      public:
      DataException(const String name) : Exception(String("Error in Message Data '") + name + "'") {}
    };

    class PropertyException : public Exception {
      public:
      PropertyException(const String name) : Exception(String("Error in Message Property ") + name + "'") {}
    };

    virtual ~Message();
    
    void addInt       (const String key, int value);
    void addBool      (const String key, bool value);
    void addString    (const String key, const String value);
    void addIntArray  (const String key, const Buffer<int> value);
    void addByteArray (const String key, const Buffer<char> value);

    void addIntProperty   (const String key, const int value);
    void addBoolProperty  (const String key, const bool property);
    void addStringProperty(const String key, const String value);

    int                getInt      (const String key) const throw(DataException);
    bool               getBool     (const String key) const throw(DataException);
    const String       getString   (const String key) const throw(DataException);
    const Buffer<int>  getIntArray (const String key) const throw(DataException);
    const Buffer<char> getByteArray(const String key) const throw(DataException);

    int          getIntProperty    (const String key) const throw(PropertyException);
    bool         getBoolProperty   (const String key) const throw(PropertyException);
    const String getStringProperty (const String key) const throw(PropertyException);

    virtual void send() const = 0;

  protected:
    Message();
    
  private:
    HashMap datas;
    HashMap properties;
};

};

#endif
