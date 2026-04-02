include config.mk
.PHONY: all deps disk_image boot kernel build_dir deps_dir clean purge

all: deps disk_image

#
# Dependencies
#
deps: deps_dir $(BOOTLOADER_BIN) $(STB_SPRINTF_H)

$(BOOTLOADER_BIN):
	@mkdir -p $(DEPS_INCLUDE)
	@wget -qO $@ $(BOOTLOADER_GIT)/releases/download/$(BOOTLOADER_VERSION)/BootLoader-MBR-i686.bin
	@wget -qO- $(BOOTLOADER_GIT)/archive/refs/tags/$(BOOTLOADER_VERSION).tar.gz | tar -xz -C $(DEPS_INCLUDE) --strip-components=2 --wildcards "*/include/*"

#
# Disk Image
#
disk_image: $(DISK_IMAGE)

PARTITION_OFFSET = $(shell expr $(BOOTLOADER_RESERVED_SECTORS) \* $(DISK_IMAGE_BS))

$(DISK_IMAGE): boot kernel
	@dd if=/dev/zero of=$@ bs=$(DISK_IMAGE_BS) count=$(DISK_IMAGE_SECTORS) >/dev/null
	@dd if=$(BOOTLOADER_BIN) of=$@ conv=notrunc >/dev/null
	@echo "$(BOOTLOADER_RESERVED_SECTORS),,b,*" | sfdisk $@ 2>/dev/null | grep "Created a new partition"

	@dd if=$(BOOT_BIN) of=$@ bs=512 seek=32 conv=notrunc >/dev/null
	@mformat -i $@@@$(PARTITION_OFFSET) -F -R $(RESERVED_SECTORS) -v "NBOS" -k ::

	@mmd -i $@@@$(PARTITION_OFFSET) "::system"
	@mcopy -i $@@@$(PARTITION_OFFSET) $(KERNEL_BIN) "::system/kernel.bin"

#
# Boot
#
boot: $(BOOT_BIN)

$(BOOT_BIN): build_dir
	@$(MAKE) -C boot BUILD_DIR=$(BUILD_DIR)
	@if [ $$(wc -c < $(BOOT_BIN)) -gt $$(( $(RESERVED_SECTORS) * 512 )) ]; then \
        echo "ERROR: boot.bin too large!"; \
        exit 1; \
    fi

#
# Kernel
#
kernel: $(KERNEL_BIN)

$(KERNEL_BIN): build_dir
	@$(MAKE) -C kernel BUILD_DIR=$(BUILD_DIR)

#
# Run
#
run:
	@qemu-system-x86_64 \
	-machine q35,smbus=off \
	-cpu qemu64 \
	-m 256M \
	-nodefaults \
	-device ich9-ahci,id=ahci \
	-drive file=$(DISK_IMAGE),id=disk0,format=raw,if=none \
	-device ide-hd,drive=disk0,bus=ahci.0 \
	-vga std -serial stdio

#
# Util
#
build_dir:
	@mkdir -p $(BUILD_DIR)
	@if [ "$(BUILD_ON_RAM)" = "1" ]; then \
		mountpoint -q $(BUILD_DIR) || sudo mount -t tmpfs -o size=$(BUILD_ON_RAM_SIZE) tmpfs $(BUILD_DIR); \
	else \
		mountpoint -q $(BUILD_DIR) && sudo umount $(BUILD_DIR) || true; \
	fi

deps_dir:
	@mkdir -p $(DEPS_BIN)
	@mkdir -p $(DEPS_INCLUDE)

clean:
	@test -n "$(BUILD_DIR)" && rm -rf $(BUILD_DIR)/*

purge:
	@test -n "$(DEPS_DIR)" && rm -rf $(DEPS_DIR)/*