ASM=nasm
CC=gcc
CC16 = /mnt/c/WATCOM/binl/wcc
LD16 = /mnt/c/WATCOM/binl/wlink

SRC_DIR=src
BUILD_DIR=build

.PHONY: all disk_image kernel bootloader clean always run

all: disk_image

#
# Disk image
#
disk_image: $(BUILD_DIR)/main_disk.img

$(BUILD_DIR)/main_disk.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main_disk.img bs=512 count=32768
	mkfs.fat -F 16 -n "NBOS" -s 1 $(BUILD_DIR)/main_disk.img

	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main_disk.img bs=1 count=3 conv=notrunc status=none
	dd if=$(BUILD_DIR)/stage1.bin of=$(BUILD_DIR)/main_disk.img bs=1 skip=62 seek=62 count=450 conv=notrunc status=none

	mcopy -i $(BUILD_DIR)/main_disk.img $(BUILD_DIR)/stage2.bin "::stage2.bin"
	mcopy -i $(BUILD_DIR)/main_disk.img $(BUILD_DIR)/kernel.bin "::kernel.bin"

#
# Bootloader
#
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	rm -rf $(BUILD_DIR)/*

#
# Run
#
run: $(BUILD_DIR)/main_disk.img
	qemu-system-i386 -drive file=$(BUILD_DIR)/main_disk.img