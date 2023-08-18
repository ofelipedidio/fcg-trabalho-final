# Compiler and Linker
CXX      := g++
CXXFLAGS := -Wall -Wno-unused-function -g -I ./include/ -o ./bin/fcg_game 
LDFLAGS  := 
LDLIBS   := 

# Files and Objects
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

all: make_directories run

bin/fcg_game: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c %< -o $@

make_directories:
	mkdir -p obj
	mkdir -p src 

./bin/fcg_game: src/main.cpp src/glad.c include/utils.h
	mkdir -p bin/Linux
	g++ -std=c++11 $(CXXFLAGS) src/glad.c src/main.cpp ./lib-linux/libglfw3.a -lrt -lm -ldl -lX11 -lpthread -lXrandr -lXinerama -lXxf86vm -lXcursor

.PHONY: clean run
clean:
	rm -f bin/Linux/main

run: ./bin/Linux/main
	cd bin/Linux && ./main

$(info $$SOURCES is [${SOURCES}])
$(info $$OBJECTS is [${OBJECTS}])

