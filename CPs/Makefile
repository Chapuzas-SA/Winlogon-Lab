# Cross-compile a Windows x64 DLL from Linux using MinGW-w64.
CXX = x86_64-w64-mingw32-g++
WINDRES = x86_64-w64-mingw32-windres
BUILD_DIR := build
TARGET := $(BUILD_DIR)/Provider.dll
SOURCES := src/Dll.cpp src/Provider.cpp
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES)) $(BUILD_DIR)/Provider.res.o

CPPFLAGS += -DUNICODE -D_UNICODE
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=c++11 -MMD -MP
LDFLAGS += -shared -s -static-libgcc -static-libstdc++ -Wl,--exclude-all-symbols
LDLIBS += -lole32 -luuid -luser32

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJECTS) Makefile
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/Provider.res.o: src/Provider.rc src/Resource.h resources/provider.bmp resources/provider.png | $(BUILD_DIR)
	$(WINDRES) -I. -Isrc -i $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
