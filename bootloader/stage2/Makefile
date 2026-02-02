TARGET_CFLAGS += -I$(SOURCE_DIR)/bootloader
TARGET_LINKFLAGS += -T linker.ld -nostdlib

HEADERS_C = 	$(shell find . -type f -name "*.h") ../BPB.h
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/bootloader/stage2/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/bootloader/stage2/asm/%.o, $(SOURCES_ASM))

.PHONY: all stage2 clean always

all: stage2

stage2: $(BUILD_DIR)/bootloader/stage2.bin

$(BUILD_DIR)/bootloader/stage2.bin: $(OBJECTS_ASM) $(OBJECTS_C)
	@mkdir -p $(@D)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/stage2.map -o $@ $^ $(TARGET_LIBS) $(ARCH_i686_LIB) $(HAL_LIB) $(LIBC)
	@echo "--> Created  stage2.bin"

$(BUILD_DIR)/bootloader/stage2/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/bootloader/stage2/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/bootloader/stage2.bin