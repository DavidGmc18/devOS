ASM=nasm

SRC_DIR=src
BUILD_DIR=build

.PHONY: all disk kernel bootloader clean always

all: disk


# Floppy image
disk: $(BUILD_DIR)/disk.img
$(BUILD_DIR)/disk.img: bootloader kernel
	dd if=/dev/zero of=$(BUILD_DIR)/disk.img bs=512 count=32768
	mkfs.fat -F 16 -n "NBOS" $(BUILD_DIR)/disk.img
	dd if=$(BUILD_DIR)/bootloader.bin of=$(BUILD_DIR)/disk.img conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(BUILD_DIR)/disk.img bs=512 seek=1 conv=notrunc
# 	mcopy -i $(BUILD_DIR)/disk.img hello.txt "::hello.txt"
# 	dd if=hello.txt of=$(BUILD_DIR)/disk.img bs=1 seek=$$((512 + $$(stat -c%s $(BUILD_DIR)/kernel.bin))) conv=notrunc


# Bootloader
bootloader: $(BUILD_DIR)/bootloader.bin
$(BUILD_DIR)/bootloader.bin: always
	$(ASM) -f bin $(SRC_DIR)/bootloader/boot.asm -o $@


# Kernel
kernel: $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.bin: always
	$(ASM) -f bin $(SRC_DIR)/kernel/main.asm -o $@


# Always
always:
	mkdir -p $(BUILD_DIR)


# Clean
clean:
	rm -rf $(BUILD_DIR)/*