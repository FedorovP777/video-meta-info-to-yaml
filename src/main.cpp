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
#include <mutex>
#include <future>

extern "C" {
#include <libavutil/common.h>
#include <libavutil/bprint.h>
}
using namespace std;
namespace fs = std::filesystem;
set<string> allowedExtensions = {".mp4", ".mxf"};

int readMetaInfo(Context &context, string file) {
  avformat_close_input(&context.cntx);
  if (avformat_open_input(&context.cntx, file.c_str(), nullptr, nullptr) < 0) {
    fprintf(stderr, "Could not open input file '%s'", file.c_str());
    return -1;
  }
  if (avformat_find_stream_info(context.cntx, nullptr) < 0) {
    fprintf(stderr, "Failed to retrieve input stream information");
    return -1;
  }
  return 0;
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

std::mutex mtx;

void endFileYaml(YAML::Emitter *&outYaml, string filename) {
  *outYaml << YAML::EndSeq; // список файлов
  std::ofstream outputFile(filename);
  outputFile << outYaml->c_str();
  outputFile.close();
  delete outYaml;
}

void handleFile(YAML::Emitter *outYaml, const filesystem::path &file, Context &context) {

  if (readMetaInfo(context, file) < 0) {
    return;
  }

  *outYaml << YAML::BeginMap; // объект файла
  setKeyValue(*outYaml, "filename", file.filename());
  setKeyValue(*outYaml, "path", file.relative_path());
  setKeyValue(*outYaml, "uuid", genUuid().str());
  setKeyValue(*outYaml, "hash", "");

  setKeyValue(*outYaml, "streams", YAML::BeginSeq); // список стримов

  for (int i = 0; i < context.cntx->nb_streams; i++) {
    AVStream *stream = context.cntx->streams[i];
    *outYaml << YAML::Value << YAML::BeginMap;  // объект стрима
    setKeyValue(*outYaml, "id", i);
    setKeyValue(*outYaml, "type", getCodecType(stream->codecpar->codec_type).str());

    if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      stringstream aspectRatio;
      AVRational display_aspect_ratio;

      av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                stream->codecpar->width * (int64_t) stream->sample_aspect_ratio.num,
                stream->codecpar->height * (int64_t) stream->sample_aspect_ratio.den,
                1024 * 1024);

      aspectRatio << display_aspect_ratio.num << ":" << display_aspect_ratio.den;
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
    *outYaml << YAML::EndMap; // объект стрима
  }
  *outYaml << YAML::EndSeq;  // список стримов
  *outYaml << YAML::EndMap;  // объект файла

}

vector<vector<int>> getRanges(int filesCount, int threads) {
  vector<vector<int>> ranges;

  int bundleSize = filesCount / threads;

  if (threads > filesCount - 1) {
    bundleSize = filesCount;
  }
  cout << "bundleSize:" << bundleSize << endl;
  int prevValue = 0;
  for (int p = bundleSize; p + bundleSize <= filesCount; p += bundleSize) {
//        cout << "{prevValue, p}:" << prevValue << "," << p << endl;
    ranges.push_back({prevValue, p});
    prevValue = p + 1;
  }
//    cout << "{prevValue, p}:" << prevValue << "," << filesCount - 1 << endl;
  ranges.push_back({prevValue, filesCount - 1});

  return ranges;
}

int main() {

  string path = "/files";
  auto allFiles = getFolderFiles(path);
  vector<filesystem::path> files;
  files.reserve(allFiles.size());

  for (const auto &file : allFiles) {
    auto fileExtension = toLowercase(file.extension());
    if (allowedExtensions.contains(fileExtension)) {
      files.push_back(file);
    }
  }
  allFiles.resize(0);
  int fileNameCounter = 0;
  cout << "Total files: " << files.size() << endl;
  int i = 0;
  int threads = 20;
  auto ranges = getRanges(files.size(), threads);
  vector<std::future<int>> tasks;
  int sizeFileYaml = 10000;

  for (auto &range:ranges) {

    tasks.push_back(std::async(std::launch::async, [&range, &files, &fileNameCounter, &i, &sizeFileYaml]() {
      int counterSizeFileYaml = 0;
      auto outYaml = new YAML::Emitter;
      *outYaml << YAML::BeginSeq; // список файлов
      Context context;

      for (int u = range[0]; u <= range[1]; u++) {

        if (counterSizeFileYaml >= sizeFileYaml) {  //  Размер файла
          endFileYaml(outYaml, getYamlFilename(fileNameCounter));
          mtx.lock();
          fileNameCounter++;
          mtx.unlock();
          outYaml = new YAML::Emitter;
          *outYaml << YAML::BeginSeq;  // список файлов
          counterSizeFileYaml = 0;
        }

        handleFile(outYaml, files[u], context);

        counterSizeFileYaml++;
        mtx.lock();
        cout << ++i << "/" << files.size() << endl;
        mtx.unlock();

      }
      endFileYaml(outYaml, getYamlFilename(fileNameCounter));
      return 0;
    }));
  }

  //Завершение потоков
  for (auto &f : tasks) {
    cout << f.get() << endl;
  }

  return 0;
}
