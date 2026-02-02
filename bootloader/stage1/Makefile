.PHONY: all clean

all: stage1

stage1: $(BUILD_DIR)/bootloader/stage1.bin

$(BUILD_DIR)/bootloader/stage1.bin: boot.asm ../BPB.inc
	@mkdir -p $(@D)
	@nasm $< -f bin -o $@
	@echo "--> Created  stage1.bin"

clean:
	@rm -f $(BUILD_DIR)/bootloader/stage1.bin