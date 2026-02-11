TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = $(shell find . -type f -name "*.h")
SOURCES_C = $(shell find . -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/system/c/%.o, $(SOURCES_C))

HEADERS_CPP = $(shell find . -type f -name "*.hpp")
SOURCES_CPP = $(shell find . -type f -name "*.cpp")
OBJECTS_CPP = $(patsubst %.cpp, $(BUILD_DIR)/system/cpp/%.o, $(SOURCES_CPP))

.PHONY: all system clean always

all: system

system: $(SYSTEM)

$(SYSTEM): $(OBJECTS_C) $(OBJECTS_CPP)
	@mkdir -p $(@D)
	@$(TARGET_AR) rcs $@ $^
	@echo "--> Created:  system.a"

$(BUILD_DIR)/system/c/%.o: %.c $(HEADERS_C) $(HEADERS_CPP)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/system/cpp/%.o: %.cpp $(HEADERS_C) $(HEADERS_CPP)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CXXFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<