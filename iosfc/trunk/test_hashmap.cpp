#include <ios_fc.h>
#include <iostream>
#include <strstream>

int main(int argc, char **argv)
{
    ios_fc::HashMap hm;
    for (int i=0; i<256; ++i) {
        std::strstream key;
        key << "concombre" << i;
        hm.put(key.str(), i);
    }
    ios_fc::HashValue *val = hm.get("concombre12");
    std::cout << val->i << std::endl;
    return 0;
}
