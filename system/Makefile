TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS = $(shell find . -type f -name "*.h")
SOURCES = $(shell find . -type f -name "*.c")

OBJECTS = $(patsubst %.c, $(BUILD_DIR)/system/%.o, $(SOURCES))

.PHONY: all system clean always

all: system

system: $(SYSTEM)

$(SYSTEM): $(OBJECTS)
	@mkdir -p $(@D)
	@$(TARGET_AR) rcs $@ $^
	@echo "--> Created:  system.a"

$(BUILD_DIR)/system/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<