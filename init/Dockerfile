# Use ubuntu:20.04 as the base image
FROM ubuntu:20.04

# Avoid warnings by switching to noninteractive
ARG DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    apt-utils \
    gcc-10 g++-10 gdb libstdc++-10-dev libc6-dev \
    llvm-10 lldb-10 libc++-10-dev \
    make ninja-build autoconf automake libtool m4 cmake ccache \
    libgtest-dev libgmock-dev libbenchmark-dev \
    build-essential \
    zlib1g-dev \
    libboost-all-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create project directory and set it as working directory
RUN mkdir -p /project
WORKDIR /project
