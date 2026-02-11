TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/boot/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/boot/asm/%.o, $(SOURCES_ASM))

.PHONY: all boot clean always

all: boot

boot: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: $(OBJECTS_ASM) $(OBJECTS_C) $(FAT_DRIVER) $(ATA_DRIVER) $(VGA_DRIVER) $(ARCH_i686_LIB) $(HAL_LIB) $(LIBC)
	@mkdir -p $(@D)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/boot.map -o $@ $^ $(TARGET_LIBS) $(FAT_DRIVER) $(ATA_DRIVER) $(VGA_DRIVER) $(ARCH_i686_LIB) $(HAL_LIB) $(LIBC)
	@echo "--> Created  boot.bin"

$(BUILD_DIR)/boot/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/boot/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/boot.bin