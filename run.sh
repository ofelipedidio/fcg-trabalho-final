#!/bin/bash
g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/Linux/main src/main.cpp src/glad.c src/textrendering.cpp ./lib-mingw-32/* -lgdi32 -lopengl32
cd bin/Linux
./main.exe