cmake_minimum_required(VERSION 3.16)
project(video_meta_reader)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
        /lib
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /usr/lib/x86_64-linux-gnu)

SET(CMAKE_CXX_FLAGS_RELEASE "-O1")
SET(CMAKE_C_FLAGS_RELEASE "-O1")

set(CMAKE_CXX_STANDARD 20)
find_package(yaml-cpp)
find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)
#
find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(LIBSWSCALE_INCLUDE_DIR libswscale/swscale.h)
find_library(SWSCALE_LIBRARY swscale)
find_library(AVUTIL_LIBRARY avutil)
set(CMAKE_BUILD_TYPE Debug)


add_executable(video_meta_reader src/main.cpp src/utils.h)
target_include_directories(video_meta_reader PRIVATE ${AVCODEC_INCLUDE_DIR} ${LIBSWSCALE_INCLUDE_DIR})
target_link_libraries(video_meta_reader PRIVATE yaml-cpp uuid pthread ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})

#################################
## PVS-Studio
#################################
#
#include(./PVS-Studio.cmake)
#
#pvs_studio_add_target(TARGET video_meta_reader_analyze ALL
#        OUTPUT FORMAT errorfile
#        ANALYZE ${PROJECT_NAME}
#        MODE GA:1,2 OP
#        LOG target.err)
