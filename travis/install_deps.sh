#!/bin/bash

set -e

if [ ! -d "$HOME/lib/$COMPILER/glfw-lib/lib" ]; then
    # No ppa for GLFW3 at this time. Compiling by hand!
    git clone --depth=100 https://github.com/glfw/glfw.git
    pushd glfw
    CMAKE_CXX_FLAGS=-fPIC CMAKE_C_FLAGS=-fPIC cmake -DCMAKE_INSTALL_PREFIX=$HOME/lib/$COMPILER/glfw-lib -DBUILD_SHARED_LIBS:bool=true . && make && make install
    popd
else
    echo "GLFW - Using cached directory";
fi

if [ ! -d "$HOME/lib/$COMPILER/glm/glm-0.9.5.3" ]; then
    #Download GLM 9.5 from github
    wget https://github.com/g-truc/glm/archive/0.9.5.3.zip
    unzip 0.9.5.3.zip -d $HOME/lib/$COMPILER/glm
else
    echo "GLM - Using cached directory";
fi

if [ ! -d "$HOME/lib/$COMPILER/gtest-release-1.7.0/include" ]; then
    # Download GTest
    wget https://github.com/google/googletest/archive/release-1.7.0.zip
    unzip googletest-release-1.7.0 -d $HOME/lib/$COMPILER
    cd $HOME/lib/$COMPILER/gtest-release-1.7.0
    cmake . && make
else
    echo "Google Test - Using cached directory";
fi

