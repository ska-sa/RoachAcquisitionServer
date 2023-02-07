FROM debian:9.13-slim

RUN apt-get update && apt-get install -y git \
    build-essential \
    cmake \
    libboost-system1.62-dev \
    libboost-thread1.62-dev \
    libboost-program-options1.62-dev \
    libboost-filesystem1.62-dev \
    libhdf5-dev \
    python2.7 \
    python-pip

RUN pip install katversion==0.9
RUN pip install katcp==0.6.2

WORKDIR /workspace
RUN git clone https://github.com/ska-sa/casperfpga.git
WORKDIR /workspace/casperfpga
RUN git checkout v0.4.3
RUN python setup.py install

WORKDIR /workspace
RUN git clone --recursive https://github.com/ska-sa/katcp_devel.git
WORKDIR /workspace/katcp_devel
# Change to KATCP v5.0
RUN git checkout version-5.0
RUN make
RUN make -C katcp install

WORKDIR /workspace
RUN git clone --recursive https://github.com/ska-sa/RoachAcquisitionServer.git
WORKDIR /workspace/RoachAcquisitionServer

RUN cmake .
RUN make
