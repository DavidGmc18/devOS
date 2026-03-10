HEADERS_C = $(shell find . -type f -name "*.h")
SOURCES_C = $(shell find . -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))

.PHONY: all kernel

all: kernel

kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@x86_64-elf-gcc -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ -lgcc
	@echo "--> Created  kernel.bin"

$(BUILD_DIR)/kernel/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@x86_64-elf-gcc -g -O2 -DDEBUG -ffreestanding -nostdlib -I. -c -o $@ $<
	@echo "--> Compiled: " $<