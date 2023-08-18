./bin/macOS/main: src/main.cpp src/glad.c include/utils.h
	mkdir -p bin/macOS
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/macOS/main src/main.cpp src/glad.c -framework OpenGL -L/usr/local/lib -lglfw -lm -ldl -lpthread

.PHONY: clean run
clean:
	rm -f bin/macOS/main

run: ./bin/macOS/main
	cd bin/macOS && ./main
