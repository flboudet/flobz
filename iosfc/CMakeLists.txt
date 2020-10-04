cmake_minimum_required(VERSION 2.8)

ADD_LIBRARY(iosfc STATIC
            ios_basemessage.cpp
            ios_serversocket.cpp
            ios_datagramsocket.cpp
            ios_socket.cpp
            ios_exception.cpp
            ios_standardmessage.cpp
            ios_fastmessage.cpp
            ios_stream.cpp
            ios_filemanager.cpp
            ios_thread.cpp
            ios_filepath.cpp
            ios_time.cpp
            ios_filestream.cpp
            ios_udpmessageboxbase.cpp
            ios_hash.cpp
            ios_udpmessage.cpp
            ios_httpdocument.cpp
            ios_igpclient.cpp
            ios_igpdatagram.cpp
            ios_igpmessagebox.cpp
            ios_igpmessage.cpp
            ios_igpmessagelistener.cpp
            ios_igpvirtualpeermessagebox.cpp
            ios_memory.cpp
            ios_messagebox.cpp
            ios_networkinterfacerequester.cpp
            ios_selector.cpp
            ios_server.cpp
           )
