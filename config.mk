export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)
export DEPS_DIR = $(abspath deps)
export DEPS_BIN = $(DEPS_DIR)/bin
export DEPS_INCLUDE = $(DEPS_DIR)/include
export LIB_DIR = $(abspath lib)

#
# Config
#
BUILD_ON_RAM = 1
BUILD_ON_RAM_SIZE = 128M

DISK_IMAGE_BS = 512
DISK_IMAGE_SECTORS = 131072 # 64MB

RESERVED_SECTORS = 32

DISK_IMAGE = $(BUILD_DIR)/diskimage.dd
export BOOT_BIN = $(BUILD_DIR)/boot.bin
export KERNEL_BIN = $(BUILD_DIR)/kernel.bin

#
# Dependencies
#
BOOTLOADER_VERSION = v0.4.0-beta
BOOTLOADER_GIT = https://github.com/DavidGmc18/BootLoader
BOOTLOADER_BIN = $(DEPS_DIR)/bin/BootLoader-MBR-i686-$(BOOTLOADER_VERSION).bin
BOOTLOADER_RESERVED_SECTORS = 32