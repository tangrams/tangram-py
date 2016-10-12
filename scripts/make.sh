#!/bin/bash

set -e

OS=$(uname)
DIST="UNKNOWN"

# Dependencies
DEPS_COMMON="cmake " 
DEPS_LINUX_COMMON="libcurl4-openssl-dev uuid-dev libtool pkg-config build-essential autoconf automake lcov"
DEPS_LINUX_RASPBIAN="curl "
DEPS_LINUX_UBUNTU="xorg-dev libgl1-mesa-dev "
DEPS_DARWIN="glfw3 pkg-config zeromq"

# Compiling
CMAKE_ARG=""
N_CORES=1

# Installing
INSTALL_FOLDER="/usr/local/bin"

# Running
PORT=8080
N_THREAD=1
# what linux distribution is?
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DIST=$NAME
fi

case "$1" in
    dependencies)
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

        # GET SUBMODULES
        echo "Installing submodules"
        git clone https://github.com/tangrams/tangram-es.git
        cd tangram-es
        git submodule update --init --recursive
        cd ..

        $0 compile $2
        ;;

    compile)
        if [ $OS == "Linux" ]; then
            echo "Preparing CMAKE for Linux - $DIST"
            N_CORES=$(grep -c ^processor /proc/cpuinfo)

            if [ "$DIST" == "Ubuntu" ]; then
                CMAKE_ARG="-DPLATFORM_TARGET=linux"

            elif [ "$DIST" == "Raspbian GNU/Linux" ]; then
                export CXX=/usr/bin/g++-4.9
                CMAKE_ARG="-DPLATFORM_TARGET=rpi"
            fi

        elif [ $OS == "Darwin" ]; then
            echo "Preparing CMAKE for Darwin"
            N_CORES="4"
            CMAKE_ARG="-DPLATFORM_TARGET=osx"
        fi
            
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
        fi

        cp 'bin/tangram-py' ../../tangram
        ;;

    clean)
        if [ ! -d worker/build ]; then
            rm -rf build
        fi        
        ;;

    *)
        $0 dependencies
        ;;
esac





