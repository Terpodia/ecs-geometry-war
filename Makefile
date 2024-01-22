CXX := g++
OUTPUT := geowar

CXX_FLAGS := -O3 -std=c++20 -Wno-unused-result
INCLUDES := -I ./src -I ./src/imgui
LDFLAGS := -O3 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL

SRC_FILES := $(wildcard src/*.cpp src/imgui/*.cpp)
OBJ_FILES := $(SRC_FILES:.cpp=.o)

all:$(OUTPUT)

$(OUTPUT):$(OBJ_FILES) Makefile 
		$(CXX) $(OBJ_FILES) $(LDFLAGS) -o ./bin/$@

.cpp.o: 
		$(CXX) -c $(CXX_FLAGS) $(INCLUDES) $< -o $@

run: $(OUTPUT) 
		cd bin && ./geowar && cd ../
