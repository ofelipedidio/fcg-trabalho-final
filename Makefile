./bin/main: src/*.cpp
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./src/ -o ./bin/main $$(find src -type d \( -name 'GLFW' -o -name 'glad' -o -name 'glm' -o -name 'KHR' \) -prune -o \( -type f -a \( -name '*.c' -o -name '*.cpp' \) \) -print) ./lib-linux/*.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run
clean:
	rm -f bin/main

run: ./bin/main
	cd bin && ./main
