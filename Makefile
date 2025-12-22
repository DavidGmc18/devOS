ASM = nasm
CC = gcc
CC16 = /mnt/c/WATCOM/binl/wcc
LD16 = /mnt/c/WATCOM/binl/wlink

MTOOLS = mcopy -i $(DISK_IMG)

SRC_DIR = src
BUILD_DIR = build

DISK_IMG = $(BUILD_DIR)/main_disk.img
IMG_SECTORS = 32768
RESERVED_SECTORS = 2

.PHONY: all disk_image kernel bootloader clean always run

all: disk_image


# Disk image
disk_image: $(DISK_IMG)
$(DISK_IMG): bootloader kernel
	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=$(IMG_SECTORS) status=none
	mkfs.fat -F 16 -f 2 -R $(RESERVED_SECTORS) -s 1 -r 256 -S 512 -n "$(IMG_NAME)" $(DISK_IMG)

	dd if=$(BUILD_DIR)/stage1.bin of=$(DISK_IMG) bs=1 count=3 conv=notrunc status=none
	dd if=$(BUILD_DIR)/stage1.bin of=$(DISK_IMG) bs=1 skip=62 seek=62 count=450 conv=notrunc status=none
# 	dd if=$(BUILD_DIR)/stage2.bin of=$(DISK_IMG) bs=512 seek=1 conv=notrunc status=none
# 	$(MTOOLS) $(BUILD_DIR)/kernel.bin "::kernel.bin"
	dd if=$(BUILD_DIR)/kernel.bin of=$(DISK_IMG) bs=512 seek=1 conv=notrunc status=none


# Bootloader
bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin
$(BUILD_DIR)/stage1.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin
$(BUILD_DIR)/stage2.bin: always
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))


# Kernel
kernel: $(BUILD_DIR)/kernel.bin
$(BUILD_DIR)/kernel.bin: always
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR))


# Always
always:
	mkdir -p $(BUILD_DIR)


# Clean
clean:
	$(MAKE) -C $(SRC_DIR)/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	$(MAKE) -C $(SRC_DIR)/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	rm -rf $(BUILD_DIR)/*

# Run
run: disk_image
	@echo "Running disk image in QEMU (boot from HD)..."
	qemu-system-i386 \
		-drive format=raw,file=$(DISK_IMG),index=0,media=disk \
		-boot c \
		-m 64M \
		-monitor stdio


# BOOTLOADER_S0_SRC 	:= $(SRC_DIR)/bootloader/stage0/bootloader.asm
# BOOTLOADER_S0_BIN 	:= $(BUILD_DIR)/bootloader_s0.bin
# BOOTLOADER_S1_SRC 	:= $(SRC_DIR)/bootloader/stage1/bootloader.asm
# BOOTLOADER_S1_BIN 	:= $(BUILD_DIR)/bootloader_s1.bin
# KERNEL_SRC     		:= $(SRC_DIR)/kernel/main.asm
# KERNEL_BIN     		:= $(BUILD_DIR)/kernel.bin
# DISK_IMG       		:= $(BUILD_DIR)/main_disk.img

# # MTools
# MTOOLS := mcopy -i $(DISK_IMG)

# # Disk image parameters
# IMG_SECTORS 		:= 32768  # 16MB
# IMG_NAME    		:= NBOS
# RESERVED_SECTORS 	:= 2

# # -----------------------------------------------------
# .PHONY: all disk_image bootloader kernel always clean run

# all: disk_image

# # -----------------------------------------------------
# # Disk image
# disk_image: $(DISK_IMG)

# $(DISK_IMG): bootloader kernel
# 	@echo "Creating empty FAT16 hard disk image..."
# 	mkdir -p $(BUILD_DIR)
# 	dd if=/dev/zero of=$(DISK_IMG) bs=512 count=$(IMG_SECTORS) status=none
# 	mkfs.fat -F 16 -f 2 -R $(RESERVED_SECTORS) -s 1 -r 256 -S 512 -n "$(IMG_NAME)" $(DISK_IMG)

# 	@echo "Copying bootloader..."
# 	dd if=$(BOOTLOADER_S0_BIN) of=$(DISK_IMG) bs=1 count=3 conv=notrunc status=none
# 	dd if=$(BOOTLOADER_S0_BIN) of=$(DISK_IMG) bs=1 skip=62 seek=62 count=450 conv=notrunc status=none
# 	dd if=$(BOOTLOADER_S1_BIN) of=$(DISK_IMG) bs=512 seek=1 conv=notrunc status=none

# 	@echo "Copying kernel..."
# 	$(MTOOLS) $(KERNEL_BIN) "::kernel.bin"

# # -----------------------------------------------------
# # Bootloader
# bootloader: $(BOOTLOADER_S0_BIN) $(BOOTLOADER_S1_BIN)

# $(BOOTLOADER_S0_BIN): always
# 	@echo "Assembling stage0 bootloader..."
# 	$(ASM) $(BOOTLOADER_S0_SRC) -f bin -o $(BOOTLOADER_S0_BIN)

# $(BOOTLOADER_S1_BIN): always
# 	@echo "Assembling stage1 bootloader..."
# 	$(ASM) $(BOOTLOADER_S1_SRC) -f bin -o $(BOOTLOADER_S1_BIN)

# # -----------------------------------------------------
# # Kernel
# kernel: $(KERNEL_BIN)

# $(KERNEL_BIN): always
# 	@echo "Assembling kernel..."
# 	$(ASM) $(KERNEL_SRC) -f bin -o $(KERNEL_BIN)

# # -----------------------------------------------------
# # Ensure build directories exist
# always:
# 	mkdir -p $(BUILD_DIR)

# # -----------------------------------------------------
# # Clean
# clean:
# 	@echo "Cleaning build directory..."
# 	@if [ -d "$(BUILD_DIR)" ]; then \
# 		rm -rf $(BUILD_DIR)/*; \
# 		echo "Build files removed."; \
# 	else \
# 		echo "Nothing to clean."; \
# 	fi

# # -----------------------------------------------------
# # Run in QEMU (boot from HD)
# run: disk_image
# 	@echo "Running disk image in QEMU (boot from HD)..."
# 	qemu-system-i386 \
# 		-drive format=raw,file=$(DISK_IMG),index=0,media=disk \
# 		-boot c \
# 		-m 64M \
# 		-monitor stdio