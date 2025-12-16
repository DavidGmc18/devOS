ASM=nasm

SRC_DIR=src
TOOLS_DIR=tools
BUILD_DIR=build

.PHONY: all disk_image kernel bootloader clean always

all: disk_image


# Floppy image
disk_image: $(BUILD_DIR)/main_disk.img

$(BUILD_DIR)/main_disk.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/main_disk.img bs=512 count=32768
	mkfs.fat -F 16 -n "NBOS" $(BUILD_DIR)/main_disk.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/main_disk.img conv=notrunc
	mcopy -i $(BUILD_DIR)/main_disk.img $(BUILD_DIR)/kernel.bin "::kernel.bin"
	mcopy -i $(BUILD_DIR)/main_disk.img hello.txt "::hello.txt"


# Bootloader
bootloader: $(BUILD_DIR)/bootloader.bin

$(BUILD_DIR)/bootloader.bin: always
	$(ASM) $(SRC_DIR)/bootloader/boot.asm -f bin -o $(BUILD_DIR)/bootloader.bin


# Kernel
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	$(ASM) $(SRC_DIR)/kernel/main.asm -f bin -o $(BUILD_DIR)/kernel.bin


# Always
always:
	mkdir -p $(BUILD_DIR)


# Clean
clean:
	rm -rf $(BUILD_DIR)/*