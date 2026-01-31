BUILD_DIR?=build/
ASM?=nasm

.PHONY: all clean

all: stage0

stage0: $(BUILD_DIR)/bootloader/stage0.bin

$(BUILD_DIR)/bootloader/stage0.bin: boot.asm ../BPB.inc
	@mkdir -p $(@D)
	@$(ASM) $< -f bin -o $@
	@echo "--> Created  stage0.bin"

clean:
	@rm -f $(BUILD_DIR)/bootloader/stage0.bin