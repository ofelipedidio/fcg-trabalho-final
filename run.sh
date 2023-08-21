#!/bin/bash
mkdir -p bin
g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/Linux/main $(find src -maxdepth 1 -type f) ./lib-mingw-32/* -lgdi32 -lopengl32
cd bin
./main.exe
