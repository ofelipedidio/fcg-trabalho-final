./bin/main: src/*.cpp include/*.h
	mkdir -p bin/Linux
	g++ -std=c++11 -Wall -Wno-unused-function -g -I ./include/ -o ./bin/main $$(find src -type f \( -name "*.c" -o -name "*.cpp" \)) ./lib-linux/*.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run
clean:
	rm -f bin/main

run: ./bin/main
	cd bin && ./main
