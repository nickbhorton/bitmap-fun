CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -g3 -Wno-unused-function -I ./include
BUILD_DIR = build
SRC_DIR = src/

all: test

build/test_images.o: src/test_images.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/test.o: src/test.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/PngByte.o: src/PngByte.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/Png.o: src/Png.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: build/test_images.o build/test.o build/PngByte.o build/Png.o
	$(CXX) $(CXXFLAGS) $(BUILD_DIR)/test_images.o $(BUILD_DIR)/PngByte.o $(BUILD_DIR)/Png.o $(BUILD_DIR)/test.o -o bin/$@
	./bin/test
	