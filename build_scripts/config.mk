export SOURCE_DIR = $(abspath .)
export BUILD_DIR = $(abspath build)
export INCLUDE_DIR = $(abspath include)

export CFLAGS = -std=c99 -g
export ASMFLAGS =
export CC = gcc
export CXX = g++
export LD = gcc
export ASM = nasm
export LINKFLAGS =
export LIBS =

export TARGET_ASMFLAGS = -f elf
export TARGET_CFLAGS = -std=c99 -g -O2 -DDEBUG -ffreestanding -nostdlib -I. -I$(SOURCE_DIR) -I$(SOURCE_DIR)/include -I$(SOURCE_DIR)/src/libs
export TARGET_CXXFLAGS = -std=c++11 -g -O2 -DDEBUG -ffreestanding -nostdlib -fno-exceptions -I. -I$(SOURCE_DIR) -I$(SOURCE_DIR)/include -I$(SOURCE_DIR)/src/libs
export TARGET_LINKFLAGS = -T linker.ld -nostdlib
export TARGET_LIBS = -lgcc

export ARCH_i686_LIB = $(BUILD_DIR)/arch/i686.a
export HAL_LIB = $(BUILD_DIR)/hal.a
export LIBC = $(BUILD_DIR)/libc.a

BINUTILS_VERSION = 2.45.1
BINUTILS_URL = https://ftp.gnu.org/gnu/binutils/binutils-$(BINUTILS_VERSION).tar.xz

GCC_VERSION = 15.2.0
GCC_URL = https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_VERSION)/gcc-$(GCC_VERSION).tar.xz

BOOTLOADER_VERSION = v0.2.1-alpha
BOOTLOADER_URL = https://github.com/DavidGmc18/BootLoader/releases/download/$(BOOTLOADER_VERSION)/BootLoader-MBR-i686.bin

# TODO make drivers portable
# TODO rename VGA?
# TODO make e9 driver? RTC? PIC?
export VGA_DRIVER = $(BUILD_DIR)/driver/vga.a
export ATA_DRIVER = $(BUILD_DIR)/driver/ata.a

export SYSTEM = $(BUILD_DIR)/system.a