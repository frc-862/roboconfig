cmake_minimum_required(VERSION 3.3)
INCLUDE(CMakeForceCompiler)
set(ARM_PREFIX arm-frc-linux-gnueabi)
set(CMAKE_SYSTEM_NAME Linux)
CMAKE_FORCE_CXX_COMPILER(${ARM_PREFIX}-g++ GNU)
CMAKE_FORCE_C_COMPILER(${ARM_PREFIX}-gcc GNU)
set(CMAKE_CXX_FLAGS "-std=c++1y -Wformat=2 -Wall -Wextra -Werror -pedantic -Wno-psabi" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g" CACHE STRING "" FORCE) # still want debugging for release?

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
