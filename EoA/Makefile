# Recovery System - Windows x64
CXX = x86_64-w64-mingw32-g++
WINDRES = x86_64-w64-mingw32-windres
CXXFLAGS = -DUNICODE -D_UNICODE -O2 -Wall -Wextra
BUILD_DIR = build
SRC_DIR = src

all: $(BUILD_DIR)/EoASysRec.exe

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/EoASysRec.resources.o: $(SRC_DIR)/EoASysRec.rc $(SRC_DIR)/EoASysRec.manifest | $(BUILD_DIR)
	$(WINDRES) -I $(SRC_DIR) $< $@

$(BUILD_DIR)/EoASysRec.exe: $(SRC_DIR)/EoASysRec.cpp $(BUILD_DIR)/EoASysRec.resources.o | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -std=c++17 -municode -mwindows -static -o $@ $^ -luser32 -lole32 -lshell32 -luuid

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
