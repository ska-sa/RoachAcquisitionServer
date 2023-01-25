FROM debian:9.13-slim

RUN apt-get update && apt-get install -y git \
    build-essential \
    cmake \
    libboost-system1.62-dev \
    libboost-thread1.62-dev \
    libboost-program-options1.62-dev \
    libboost-filesystem1.62-dev \
    libhdf5-dev

WORKDIR /workspace
RUN git clone --recursive https://github.com/ska-sa/katcp_devel.git
WORKDIR /workspace/katcp_devel
# This is hacky and won't work if the Makefile.inc file changes for some reason,
# but it'll change the KATCP version compiled from 4.9 to 5.0:
RUN sed -i '58s/.*/CFLAGS += -DKATCP_PROTOCOL_MAJOR_VERSION=5 -DKATCP_PROTOCOL_MINOR_VERSION=0/' Makefile.inc
RUN make
RUN make -C katcp install

WORKDIR /workspace
RUN true
RUN git clone --recursive https://github.com/ska-sa/RoachAcquisitionServer.git
WORKDIR /workspace/RoachAcquisitionServer

RUN cmake .
RUN make
