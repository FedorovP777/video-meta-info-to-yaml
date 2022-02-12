FROM ubuntu:20.04
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Moscow

RUN apt-get update -y
RUN apt-get install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release \
    build-essential \
    software-properties-common \
    cmake \
    g++ \
    wget \
    unzip \
    libavcodec-dev \
    libavformat-dev \
    libavdevice-dev \
    libavfilter-dev \
    libyaml-cpp-dev \
    zlib1g-dev \
    uuid-dev

RUN cmake --build build --target video_meta_reader