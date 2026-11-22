CXX = x86_64-w64-mingw32-g++
BUILD_DIR := build
TARGET := $(BUILD_DIR)/PreAuthPLAP.dll
SOURCES := src/dllmain.cpp src/ClassFactory.cpp src/PreAuthPLAPProvider.cpp src/PreAuthPLAPCredential.cpp
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

CPPFLAGS += -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_WIN32_WINNT=0x0A00
CXXFLAGS += -std=c++17 -Os -flto -ffunction-sections -fdata-sections -fno-rtti -fno-exceptions -DNDEBUG -Wall -Wextra -Wpedantic -MMD -MP
LDFLAGS += -shared -Os -flto -static -static-libgcc -static-libstdc++ -Wl,--gc-sections,--strip-all,--dynamicbase,--nxcompat,--high-entropy-va
LDLIBS += -lole32 -luser32 -lshell32 -luuid

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(OBJECTS) src/PreAuthPLAP.def Makefile
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) src/PreAuthPLAP.def $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp Makefile | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
