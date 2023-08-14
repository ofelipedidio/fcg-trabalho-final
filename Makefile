PROJECT  := fcp_game

# Compiler and Linker
CXX      := g++
CXXFLAGS := -Wall -Iinclude
LDFLAGS  := 
LDLIBS   := 

# Directories
SRCDIR   := src
OBJDIR   := obj
BINDIR   := bin

# Files and Objects
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET   := $(BINDIR)/$(PROJECT)


all: directories $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

directories: $(OBJDIR) $(BINDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(TARGET)

.PHONY: all clean

