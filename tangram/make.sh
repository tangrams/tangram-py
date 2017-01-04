#!/bin/bash

set -e

OS=$(uname)
ARQ=$(uname -m)
DIST="UNKNOWN"

# Dependencies
DEPS_COMMON="cmake swig " 
DEPS_LINUX_COMMON="build-essential g++ make libtool pkg-config autoconf automake lcov libcurl4-openssl-dev python-dev python3-dev uuid-dev "
DEPS_LINUX_RASPBIAN="curl "
DEPS_LINUX_UBUNTU="xorg-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libgl1-mesa-dev "
DEPS_DARWIN="glfw3 pkg-config "

# Compiling
CMAKE_ARG=""
N_CORES=1
LIB_POSFIX="libtangram-$OS-$ARQ.a"

# what linux distribution is?
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DIST=$NAME
fi

# INSTALL DEPENDECIES
if [ $OS == "Linux" ]; then
    echo "Install dependeces for Linux - $DIST"
    if [ "$DIST" == "Ubuntu" ]; then
         # UBUNTU
        sudo apt-get install $DEPS_COMMON $DEPS_LINUX_COMMON $DEPS_LINUX_UBUNTU
    elif [ "$DIST" == "Raspbian GNU/Linux" ]; then
        # Raspian
        sudo apt-get install $DEPS_COMMON $DEPS_LINUX_COMMON $DEPS_LINUX_RASPBIAN
    fi

elif [ $OS == "Darwin" ]; then
    echo "Install dependeces for Darwin"
    # DARWIN
    if [ ! -e /usr/local/bin/brew ]; then
        ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
    fi
    brew install $DEPS_COMMON $DEPS_DARWIN
fi

git submodule update --init --recursive

if [ $OS == "Linux" ]; then
    echo "Preparing CMAKE for Linux - $DIST"
    N_CORES=$(grep -c ^processor /proc/cpuinfo)

    if [ "$DIST" == "Ubuntu" ]; then
        CMAKE_ARG="-DPLATFORM_TARGET=linux"

    elif [ "$DIST" == "Raspbian GNU/Linux" ]; then
        export CXX=/usr/bin/g++-4.9
        CMAKE_ARG="-DPLATFORM_TARGET=rpi"
        N_CORES=1
    fi

elif [ $OS == "Darwin" ]; then
    echo "Preparing CMAKE for Darwin"
    N_CORES="4"
    CMAKE_ARG="-DPLATFORM_TARGET=osx"
fi

# Make SWIG wrap 
# swig -c++ -python -o src/tangram_wrap.cpp src/tangram.i 
    
if [ "$2" == "xcode" ]; then
    echo "Making XCode project"
    mkdir build
    cd build
    cmake .. -GXcode -DPLATFORM_TARGET=osx
else
    echo "Compiling worker"
    cmake . -Bbuild $CMAKE_ARG
    cd build
    make -j $N_CORES
    cd ..
fi

cp build/libtangram.a ./$LIB_POSFIX
# cp build/_tangram.so .
# cp build/tangram.py ./TangramMap.py
