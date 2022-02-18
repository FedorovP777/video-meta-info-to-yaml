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
set <string> allowedExtensions = {".mp4", ".mxf"};

void readMetaInfo(Context &context, string file) {
    if (avformat_open_input(&context.cntx, file.c_str(), nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open input file '%s'", file.c_str());
    }
    if (avformat_find_stream_info(context.cntx, nullptr) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
    }
//    av_dump_format(context.cntx, 0, file.c_str(), 0);
}

template<typename T1, typename T2>
void setKeyValue(YAML::Emitter &out, T1 key, T2 value) {
    out << YAML::Key << key;
    out << YAML::Value << value;
}

string getYamlFilename(int i) {
    stringstream fileName;
    fileName << "/output/metadata" << i << ".yaml";
    return fileName.str();
}

void endFileYaml(YAML::Emitter *outYaml, string filename) {
    *outYaml << YAML::EndSeq;
    stringstream fileName;
    std::ofstream outputFile(filename);
    outputFile << outYaml->c_str();
    outputFile.close();
    delete outYaml;
    outYaml = new YAML::Emitter;
    *outYaml << YAML::BeginSeq;
}

int main() {

    string path = "/files";
    auto files = getFolderFiles(path);
    int sizeFileYaml = 1000;
    int counterSizeFileYaml = 0;
    auto outYaml = new YAML::Emitter;
    int fileNameCounter = 0;
    *outYaml << YAML::BeginSeq;
    cout << "Total files: " << files.size() << endl;
    int i = 0;
    
    for (const auto &file : files) {

        auto fileExtension = toLowercase(file.extension());
        stringstream hashFile;
        hashFile << std::hex << std::uppercase << std::setw(16) << fs::hash_value(file);

        if (!allowedExtensions.contains(fileExtension)) {
            continue;
        }

        if (counterSizeFileYaml >= sizeFileYaml) {
            endFileYaml(outYaml, getYamlFilename(fileNameCounter));
            fileNameCounter++;
            outYaml = new YAML::Emitter;
            *outYaml << YAML::BeginSeq;
            counterSizeFileYaml = 0;
        }
        counterSizeFileYaml++;

        *outYaml << YAML::BeginMap;
        setKeyValue(*outYaml, "filename", file.filename());
        setKeyValue(*outYaml, "path", file.relative_path());
        setKeyValue(*outYaml, "uuid", genUuid().str());
        setKeyValue(*outYaml, "hash", hashFile.str());

        Context context;
        readMetaInfo(context, file);

        setKeyValue(*outYaml, "streams", YAML::BeginSeq);

        for (int i = 0; i < context.cntx->nb_streams; i++) {
            AVStream *stream = context.cntx->streams[i];
            *outYaml << YAML::Value << YAML::BeginMap;
            setKeyValue(*outYaml, "id", i);
            setKeyValue(*outYaml, "type", getCodecType(stream->codecpar->codec_type).str());

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                stringstream aspectRatio;
                aspectRatio << stream->display_aspect_ratio.num << ":" << stream->display_aspect_ratio.den;
                setKeyValue(*outYaml, "codec", getCodecName(stream->codecpar->codec_id).str());
                setKeyValue(*outYaml, "width", stream->codecpar->width);
                setKeyValue(*outYaml, "height", stream->codecpar->height);
                setKeyValue(*outYaml, "aspect_ratio", aspectRatio.str());
                setKeyValue(*outYaml, "fps", stream->r_frame_rate.num);
                setKeyValue(*outYaml, "bitrate", stream->codecpar->bit_rate);
                setKeyValue(*outYaml, "pix_format", getPixFormatName(stream->codecpar->format).str());
                setKeyValue(*outYaml, "duration", stream->duration);
            }

            if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                setKeyValue(*outYaml, "codec", getCodecName(stream->codecpar->codec_id).str());
                setKeyValue(*outYaml, "bitrate", stream->codecpar->bit_rate);
                setKeyValue(*outYaml, "channels", getAudioChannels(stream->codecpar));
            }
            *outYaml << YAML::EndMap;
        }
        *outYaml << YAML::EndSeq;
        *outYaml << YAML::EndMap;
        cout  << ++i << "/" << files.size() << endl;
    }
    stringstream fileName;
    endFileYaml(outYaml, getYamlFilename(fileNameCounter));
    return 0;
}
