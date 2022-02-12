// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <iostream>
#include <filesystem>
#include <string>
#include "Context.h"
#include "utils.h"
#include "yaml-cpp/yaml.h"
#include <uuid/uuid.h>
#include <fstream>

extern "C" {
#include <libavutil/common.h>
#include <libavutil/bprint.h>
}
using namespace std;
namespace fs = std::filesystem;
set<string> allowedExtensions = {".mp4", ".mxf"};

void readMetaInfo(Context &context, string file) {
    if (avformat_open_input(&context.cntx, file.c_str(), nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open input file '%s'", file.c_str());
    }
    if (avformat_find_stream_info(context.cntx, nullptr) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
    }
    av_dump_format(context.cntx, 0, file.c_str(), 0);
}

template<typename T1, typename T2>
void setKeyValue(YAML::Emitter &out, T1 key, T2 value) {
    out << YAML::Key << key;
    out << YAML::Value << value;
}

int main() {

    string path = "../files";
    auto files = getFolderFiles(path);
    stringstream outputstr;
    YAML::Emitter out;
    out << YAML::BeginSeq;

    for (const auto &file : files) {
        auto fileExtension = toLowercase(file.extension());
        stringstream hashFile;
        hashFile << std::hex << std::uppercase << std::setw(16) << fs::hash_value(file);
        if (!allowedExtensions.contains(fileExtension)) {
            continue;
        }
        out << YAML::BeginMap;
        setKeyValue(out, "filename", file.filename());
        setKeyValue(out, "path", file.relative_path());
        setKeyValue(out, "uuid", genUuid().str());
        setKeyValue(out, "hash", hashFile.str());

        Context context;
        readMetaInfo(context, file);

        setKeyValue(out, "streams", YAML::BeginSeq);

        for (int i = 0; i < context.cntx->nb_streams; i++) {
            AVStream *stream = context.cntx->streams[i];
            out << YAML::Value << YAML::BeginMap;
            setKeyValue(out, "id", i);
            setKeyValue(out, "type", getCodecType(stream->codecpar->codec_type).str());

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                stringstream aspectRatio;
                aspectRatio << stream->display_aspect_ratio.num << ":" << stream->display_aspect_ratio.den;
                setKeyValue(out, "codec", getCodecName(stream->codecpar->codec_id).str());
                setKeyValue(out, "width", stream->codecpar->width);
                setKeyValue(out, "height", stream->codecpar->height);
                setKeyValue(out, "aspect_ratio", aspectRatio.str());
                setKeyValue(out, "fps", stream->r_frame_rate.num);
                setKeyValue(out, "bitrate", stream->codecpar->bit_rate);
                setKeyValue(out, "pix_format", getPixFormatName(stream->codecpar->format).str());
                setKeyValue(out, "duration", stream->duration);
            }

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                setKeyValue(out, "codec", getCodecName(stream->codecpar->codec_id).str());
                setKeyValue(out, "bitrate", stream->codecpar->bit_rate);
                setKeyValue(out, "channels", getAudioChannels(stream->codecpar));
            }
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;
    std::ofstream outputFile("output.yaml");
    outputFile << out.c_str();
    outputFile.close();
    return 0;
}
