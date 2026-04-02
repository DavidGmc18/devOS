# HEADERS_C = $(shell find . -type f -name "*.h")
HEADERS_C = $(shell find $(SUB_SYSTEMS) -type f -name "*.h")
SOURCES_C = $(shell find . -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/boot/c/%.o, $(SOURCES_C))

HEADERS_ASM = $(shell find . -type f -name "*.inc")
SOURCES_ASM = $(shell find . -type f -name "*.asm")
OBJECTS_ASM = $(patsubst %.asm, $(BUILD_DIR)/boot/asm/%.o, $(SOURCES_ASM))

.PHONY: all boot

all: boot

boot: $(BOOT_BIN)

$(BOOT_BIN): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@i686-elf-gcc -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/boot.map -o $@ $^ -lgcc
	@echo "--> Created  boot.bin"

$(BUILD_DIR)/boot/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@i686-elf-gcc -g -O2 -DDEBUG -ffreestanding -nostdlib -I. -I$(SOURCE_DIR) -I$(DEPS_INCLUDE) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/boot/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@nasm -f elf32 -o $@ $<
	@echo "--> Compiled: " $<