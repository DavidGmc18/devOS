include build_scripts/config.mk

.PHONY: all floppy_image kernel bootloader clean always

all: floppy_image

include build_scripts/toolchain.mk

#
# Floppy image
#
floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@dd if=/dev/zero of=$@ bs=512 count=8192 >/dev/null
	@mkfs.fat -F 16 -R 2 -s 1 -n "NBOS" $@ >/dev/null

	@dd if=$(BUILD_DIR)/bootloader/stage0.bin of=$@ bs=1 count=11 conv=notrunc >/dev/null
	@dd if=$(BUILD_DIR)/bootloader/stage0.bin of=$@ bs=1 skip=43 seek=43 conv=notrunc >/dev/null

	@dd if=$(BUILD_DIR)/bootloader/stage1.bin of=$@ seek=1 bs=512 conv=notrunc

	@mcopy -i $@ $(BUILD_DIR)/bootloader/stage2.bin "::stage2.bin"
	@mcopy -i $@ $(BUILD_DIR)/kernel.bin "::kernel.bin"
	@echo "--> Created: " $@

#
# Bootloader
#
bootloader: stage0 stage1 stage2

stage0: $(BUILD_DIR)/bootloader/stage0.bin

$(BUILD_DIR)/bootloader/stage0.bin: always
	@$(MAKE) -C src/bootloader/stage0 BUILD_DIR=$(abspath $(BUILD_DIR))

stage1: $(BUILD_DIR)/bootloader/stage1.bin

$(BUILD_DIR)/bootloader/stage1.bin: always
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/bootloader/stage2.bin

$(BUILD_DIR)/bootloader/stage2.bin: always
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*