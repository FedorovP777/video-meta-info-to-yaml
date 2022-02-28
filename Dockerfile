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

    # INSTALL PVS-Studio
RUN wget -q -O - https://files.pvs-studio.com/etc/pubkey.txt | apt-key add - \
     && wget -O /etc/apt/sources.list.d/viva64.list \
        https://files.pvs-studio.com/etc/viva64.list \
     && apt update -yq \
     && apt install -yq pvs-studio strace \
     && pvs-studio --version \
     && apt clean -yq \
RUN pvs-studio-analyzer credentials PVS-Studio Free FREE-FREE-FREE-FREE
WORKDIR /app
RUN mkdir build && mkdir /output
RUN pwd
COPY src src
ADD CMakeLists.txt .
ADD PVS-Studio.cmake .
RUN pwd && ls -la . && ls -la ./src
RUN cd build && cmake -DCMAKE_BUILD_TYPE=Release /app
RUN cmake --build /app/build --target video_meta_reader
CMD /app/build/video_meta_reader