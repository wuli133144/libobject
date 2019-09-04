#! /bin/bash
rm -rf CMakeFiles  CMakeCache.txt    cmake_install.cmake  Makefile
cmake . -DPLATFORM=linux -DMODULE_UI=OFF
make
make install
