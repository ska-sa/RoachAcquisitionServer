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
RUN make
RUN make -C katcp install

WORKDIR /workspace
RUN git clone --recursive https://github.com/ska-sa/RoachAcquisitionServer.git
WORKDIR /workspace/RoachAcquisitionServer

RUN cmake .
RUN make
