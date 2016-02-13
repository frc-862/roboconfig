cmake_minimum_required(VERSION 3.3)
INCLUDE(CMakeForceCompiler)

if($ENV{ARM})

set(ARM_PREFIX arm-frc-linux-gnueabi)
set(CMAKE_SYSTEM_NAME Linux)
CMAKE_FORCE_CXX_COMPILER(${ARM_PREFIX}-g++ GNU)
CMAKE_FORCE_C_COMPILER(${ARM_PREFIX}-gcc GNU)
set(CMAKE_CXX_FLAGS "-std=c++1y -Wformat=2 -Wall -Wextra -Werror -pedantic -Wno-psabi" CACHE STRING "" FORCE)

else($ENV{ARM})

set(CMAKE_CXX_FLAGS "-std=c++1y -Wformat=2 -Wall -Wextra -Werror -pedantic" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-Os -s" CACHE STRING "" FORCE) # still want debugging for release?

endif($ENV{ARM})

project(roboconfig)

include_directories("${CMAKE_SOURCE_DIR}/src/inc")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -W -Wall -Wno-deprecated-register")

set(SOURCE_FILES
        src/planner.cpp
        src/segment.cpp
        src/trajectory.cpp
        
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
        src/json_trajectory.cpp
        )

add_executable(roboconfig ${SOURCE_FILES})

add_custom_target(copy_files)
add_custom_command(TARGET copy_files POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
  make_directory config)
add_custom_command(TARGET copy_files POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
  make_directory public)
add_custom_command(TARGET copy_files POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
  copy_directory ${CMAKE_SOURCE_DIR}/config config)
add_custom_command(TARGET copy_files POST_BUILD COMMAND ${CMAKE_COMMAND} -E 
  copy_directory ${CMAKE_SOURCE_DIR}/public public)
