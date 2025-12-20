# =====================================================
# Project Makefile - FAT16 OS (NASM + Kernel)
# =====================================================

ASM       := nasm
CC        := gcc
SRC_DIR   := src
BUILD_DIR := build

BOOTLOADER_SRC := $(SRC_DIR)/bootloader/bootloader.asm
BOOTLOADER_BIN := $(BUILD_DIR)/bootloader.bin
SECOND_BOOTLOADER_SRC := $(SRC_DIR)/bootloader/second_bootloader.asm
SECOND_BOOTLOADER_BIN := $(BUILD_DIR)/second_bootloader.bin
KERNEL_SRC     := $(SRC_DIR)/kernel/main.asm
KERNEL_BIN     := $(BUILD_DIR)/kernel.bin
DISK_IMG       := $(BUILD_DIR)/main_disk.img

# MTools
MTOOLS := mcopy -i $(DISK_IMG)

# Disk image parameters
IMG_SECTORS 	:= 32768  # 16MB
IMG_NAME    		:= NBOS
RESERVED_SECTORS 	:= 2

# -----------------------------------------------------
.PHONY: all disk_image bootloader kernel always clean run

all: disk_image

# -----------------------------------------------------
# Disk image
disk_image: $(DISK_IMG)

$(DISK_IMG): bootloader kernel
	@echo "Creating empty FAT16 hard disk image..."
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=$(IMG_SECTORS) status=none

	@echo "Formatting image with FAT16..."
	mkfs.fat -F 16 -f 2 -R $(RESERVED_SECTORS) -s 1 -r 256 -S 512 -n "$(IMG_NAME)" $(DISK_IMG)

	@echo "Copying bootloader..."
	dd if=$(BOOTLOADER_BIN) of=$(DISK_IMG) bs=1 count=3 conv=notrunc status=none
	dd if=$(BOOTLOADER_BIN) of=$(DISK_IMG) bs=1 skip=62 seek=62 count=450 conv=notrunc status=none
	dd if=$(SECOND_BOOTLOADER_BIN) of=$(DISK_IMG) bs=512 seek=1 conv=notrunc status=none

	@echo "Copying kernel..."
	$(MTOOLS) $(KERNEL_BIN) "::kernel.bin"

# -----------------------------------------------------
# Bootloader
bootloader: $(BOOTLOADER_BIN) $(SECOND_BOOTLOADER_BIN)

$(BOOTLOADER_BIN): always
	@echo "Assembling bootloader..."
	$(ASM) $(BOOTLOADER_SRC) -f bin -o $(BOOTLOADER_BIN)

$(SECOND_BOOTLOADER_BIN): always
	@echo "Assembling second_bootloader..."
	$(ASM) $(SECOND_BOOTLOADER_SRC) -f bin -o $(SECOND_BOOTLOADER_BIN)

# -----------------------------------------------------
# Kernel
kernel: $(KERNEL_BIN)

$(KERNEL_BIN): always
	@echo "Assembling kernel..."
	$(ASM) $(KERNEL_SRC) -f bin -o $(KERNEL_BIN)

# -----------------------------------------------------
# Ensure build directories exist
always:
	mkdir -p $(BUILD_DIR)

# -----------------------------------------------------
# Clean
clean:
	@echo "Cleaning build directory..."
	@if [ -d "$(BUILD_DIR)" ]; then \
		rm -rf $(BUILD_DIR)/*; \
		echo "Build files removed."; \
	else \
		echo "Nothing to clean."; \
	fi

# -----------------------------------------------------
# Run in QEMU (boot from HD)
run: disk_image
	@echo "Running disk image in QEMU (boot from HD)..."
	qemu-system-i386 \
		-drive format=raw,file=$(DISK_IMG),index=0,media=disk \
		-boot c \
		-m 64M \
		-monitor stdio