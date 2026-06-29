CXX = g++
SFML_INCLUDE = ./SFML/include
SFML_LIB = ./SFML/lib

CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude -I"$(SFML_INCLUDE)"
LDFLAGS = -L"$(SFML_LIB)" -lsfml-graphics -lsfml-window -lsfml-system

SRCS = src/main.cpp src/QuadTree.cpp src/Simulation.cpp src/App.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = quadtree_sim

.PHONY: all clean benchmark

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q src\*.o 2>NUL
	del /Q $(TARGET).exe benchmark_only.exe 2>NUL

benchmark: src/QuadTree.cpp src/Simulation.cpp scripts/benchmark_only.cpp
	$(CXX) $(CXXFLAGS) -o benchmark_only $^
