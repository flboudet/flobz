cmake_minimum_required(VERSION 3.5)

add_library(iosfc_posix OBJECT
    ios_unixdatagramsocketimpl.cpp
    ios_unixsocketaddressimpl.cpp
    ios_unixsocketimpl.cpp
    ios_unixthreadimpl.cpp
    ios_unixmuteximpl.cpp
    ios_unixselectorimpl.cpp
    ios_unixnetworkinterfacerequesterimpl.cpp
)

target_include_directories(iosfc_posix PRIVATE ${PROJECT_SOURCE_DIR})