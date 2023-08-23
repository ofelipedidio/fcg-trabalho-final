#!/bin/bash
mkdir -p bin
g++ -std=c++11 -Wall -Wno-unused-function -g -I ./src/ -o ./bin/main $(find src -type d \( -name 'GLFW' -o -name 'glad' -o -name 'glm' -o -name 'KHR' \) -prune -o \( -type f -a \( -name '*.c' -o -name '*.cpp' \) \) -print) ./lib-mingw-32/* -lgdi32 -lopengl32
cd bin
./main.exe
