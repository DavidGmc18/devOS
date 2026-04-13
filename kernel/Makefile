SUB_SYSTEMS = $(SOURCE_DIR)/kernel $(SOURCE_DIR)/arch $(SOURCE_DIR)/driver $(SOURCE_DIR)/lib $(SOURCE_DIR)/mm $(INCLUDE_DIR)

HEADERS_C = $(shell find $(SUB_SYSTEMS) -type f -name "*.h")
SOURCES_C = $(shell find $(SUB_SYSTEMS) -type f -name "*.c")
OBJECTS_C = $(patsubst %.c, $(BUILD_DIR)/c/%.o, $(SOURCES_C))

.PHONY: all kernel

all: kernel

kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@x86_64-elf-gcc -T linker.ld -nostdlib -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ -lgcc
	@echo "--> Created  kernel.bin"

$(BUILD_DIR)/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@x86_64-elf-gcc -g -O2 -DDEBUG -ffreestanding -mgeneral-regs-only -mcmodel=kernel -fno-pic -fno-pie -nostdlib -mno-red-zone -fstack-usage -I. -I$(SOURCE_DIR) -I$(INCLUDE_DIR) -I$(DEPS_INCLUDE) -c -o $@ $<
	@echo "--> Compiled: " $<