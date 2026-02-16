HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/kernel/asm/%.o, $(SOURCES_ASM))

HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")
OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/kernel/c/%.o, $(SOURCES_C))

HEADERS_CPP = 	$(shell find . -type f -name "*.hpp")
SOURCES_CPP = 	$(shell find . -type f -name "*.cpp")
OBJECTS_CPP = 	$(patsubst %.cpp, $(BUILD_DIR)/kernel/cpp/%.o, $(SOURCES_CPP))

.PHONY: all kernel clean always

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: $(OBJECTS_ASM) $(OBJECTS_C) $(OBJECTS_CPP) $(FAT_DRIVER) $(ATA_DRIVER) $(VGA_DRIVER) $(ARCH_i686_LIB) $(HAL_LIB) $(LIBC) $(SYSTEM)
	@$(TARGET_LD) $(TARGET_LINKFLAGS) -Wl,-Map=$(BUILD_DIR)/kernel.map -o $@ $^ $(TARGET_LIBS) $(FAT_DRIVER) $(ATA_DRIVER) $(VGA_DRIVER) $(ARCH_i686_LIB) $(HAL_LIB) $(LIBC) $(SYSTEM)
	@echo "--> Created:  kernel.bin"

$(BUILD_DIR)/kernel/asm/%.o: %.asm $(HEADERS_ASM)
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernel/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

$(BUILD_DIR)/kernel/cpp/%.o: %.cpp $(HEADERS_CPP)
	@mkdir -p $(@D)
	@$(TARGET_CXX) $(TARGET_CXXFLAGS) -c -o $@ $<
	@echo "--> Compiled: " $<

clean:
	@rm -f $(BUILD_DIR)/kernel.bin