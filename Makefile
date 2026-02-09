include build_scripts/config.mk

.PHONY: all disk_image boot kernel clean always

all: disk_image

include build_scripts/toolchain.mk

#
# Disk image
#
disk_image: $(BUILD_DIR)/diskimage.dd

$(BUILD_DIR)/diskimage.dd: deps boot kernel BootLoader-MBR-i686.bin
#	Create image with MBR BootLoader
	@dd if=/dev/zero of=$@ bs=512 count=8192 >/dev/null
	@dd if=BootLoader-MBR-i686.bin of=$@ conv=notrunc >/dev/null

# 	Create Partition
	@dd if=/dev/zero of=$(BUILD_DIR)/partition.img bs=512 count=8160 >/dev/null
	@mkfs.fat -F 16 -R 32 -s 1 -n "NBOS" $(BUILD_DIR)/partition.img >/dev/null

	@dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/partition.img conv=notrunc >/dev/null
# 	@dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/partition.img bs=1 count=11 conv=notrunc >/dev/null
# 	@dd if=$(BUILD_DIR)/boot.bin of=$(BUILD_DIR)/partition.img bs=1 skip=43 seek=43 conv=notrunc >/dev/null

# 	@mcopy -i $@ $(BUILD_DIR)/partition.img "::kernel.bin"

# 	Copy partition and set MBR
	@dd if=$(BUILD_DIR)/partition.img of=$@ bs=512 seek=32 conv=notrunc
	@echo '80' | xxd -r -p | dd of=build/diskimage.dd bs=1 seek=446 conv=notrunc
	@echo '20' /| xxd -r -p | dd of=build/diskimage.dd bs=1 seek=454 conv=notrunc
	@echo '1FE0' /| xxd -r -p | dd of=build/diskimage.dd bs=1 seek=458 conv=notrunc
# TODO is this correct partition size?

	@echo "--> Created: " $@


#
# Dependencies
#
deps:
	@$(MAKE) -C $(SOURCE_DIR)/arch/i686 BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/hal BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/lib BUILD_DIR=$(abspath $(BUILD_DIR))
	@$(MAKE) -C $(SOURCE_DIR)/driver BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Boot
#
boot: $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/boot.bin: always
	@$(MAKE) -C boot BUILD_DIR=$(abspath $(BUILD_DIR))


#
# Kernel
#
kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	@$(MAKE) -C kernel BUILD_DIR=$(abspath $(BUILD_DIR))

#
# Always
#
always:
	@mkdir -p $(BUILD_DIR)

#
# Clean
#
clean:
	@$(MAKE) -C kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*