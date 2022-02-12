// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#ifndef VIDEO_META_READER_UTILS_H
#define VIDEO_META_READER_UTILS_H
using namespace std;

#include <sstream>
#include <string>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <fmt/core.h>
#include <uuid/uuid.h>

extern "C" {

#include <libavutil/common.h>
#include <libavutil/bprint.h>

#include <libavutil/pixdesc.h>
}
namespace fs = std::filesystem;


string toLowercase(string s) {
    stringstream ss;
    locale loc;
    for (auto &i:s) {
        ss << tolower(i, loc);
    }
    return ss.str();
}

vector<filesystem::path> getFolderFiles(string folder) {
    vector <filesystem::path> result;

    for (const auto &entry : fs::recursive_directory_iterator(folder)) {
        if (entry.is_directory()) {
            continue;
        }
        result.emplace_back(std::filesystem::path(entry.path()));
    }
    return result;
}


stringstream getCodecType(AVMediaType codecType) {
    stringstream ss;
    ss << av_get_media_type_string(codecType);
    return ss;
}

stringstream getCodecName(AVCodecID codecId) {
    stringstream ss;
    const char *codec_name;
    codec_name = avcodec_get_name(codecId);
    ss << codec_name;
    return ss;
}

stringstream getPixFormatName(int formatId) {
    stringstream ss;
    ss << av_get_pix_fmt_name(static_cast<AVPixelFormat>(formatId));
    return ss;
}

int getAudioChannels(AVCodecParameters *codecpar) {
    AVCodecContext *avctx;
    avctx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(avctx, codecpar);
    return avctx->channels;
}


stringstream genUuid() {
    uuid_t uuidGenerated;
    char uuidBuff[36];
    stringstream ss;
    uuid_generate_random(uuidGenerated);
    uuid_unparse(uuidGenerated, uuidBuff);
    ss << uuidBuff;
    return ss;
}

#endif //VIDEO_META_READER_UTILS_H
