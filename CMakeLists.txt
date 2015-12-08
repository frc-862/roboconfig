cmake_minimum_required(VERSION 3.3)
project(roboconfig)

include_directories("${CMAKE_SOURCE_DIR}/src/inc")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -W -Wall -Wno-deprecated-register")

set(SOURCE_FILES
        src/inc/json/json-forwards.h
        src/inc/json/json.h
        src/jsoncpp.cpp
        src/mongoose.cpp
        src/inc/mongoose.h
        src/inc/tinydir.h
        src/config.cpp
        src/inc/config.h
        src/server.cpp
        src/inc/server.h
        src/roboconfig.cpp
        src/inc/ws_file_tail.h
        src/ws_file_tail.cpp
        )

add_executable(roboconfig ${SOURCE_FILES})