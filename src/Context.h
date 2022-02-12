// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifndef LIBAV_TEST_CONTEXT_H
#define LIBAV_TEST_CONTEXT_H

#include <sstream>
#include <string>
#include <iostream>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
#include <libavutil/common.h>
}

class Context {
public:
    AVFormatContext *cntx;

    Context() {
        cntx = avformat_alloc_context();
    }

    ~Context() {
        avformat_close_input(&cntx);
        avformat_free_context(cntx);
    }
};


#endif //LIBAV_TEST_CONTEXT_H
