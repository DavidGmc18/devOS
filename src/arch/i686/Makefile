TARGET_ASMFLAGS += -f elf
TARGET_CFLAGS += -ffreestanding -nostdlib -I.
TARGET_LIBS += -lgcc

HEADERS_C = 	$(shell find . -type f -name "*.h")
SOURCES_C = 	$(shell find . -type f -name "*.c")

HEADERS_ASM = 	$(shell find . -type f -name "*.inc")
SOURCES_ASM = 	$(shell find . -type f -name "*.asm")

OBJECTS_C = 	$(patsubst %.c, $(BUILD_DIR)/arch/i686/c/%.o, $(SOURCES_C))
OBJECTS_ASM = 	$(patsubst %.asm, $(BUILD_DIR)/arch/i686/asm/%.o, $(SOURCES_ASM))

.PHONY: all i686 clean

all: i686

i686: $(BUILD_DIR)/arch/i686.a

$(BUILD_DIR)/arch/i686.a: $(OBJECTS_ASM) $(OBJECTS_C) $(SOURCE_DIR)/src/arch/i686/isrs_gen.c
	@$(TARGET_AR) rcs $@ $^
# 	@$(TARGET_RANLIB) $@
	@echo "--> Created:  libkstd.a"

$(BUILD_DIR)/arch/i686/c/%.o: %.c $(HEADERS_C)
	@mkdir -p $(@D)
	@$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $< $(TARGET_LIBS)
	@echo "--> Compiled: " $<

$(BUILD_DIR)/arch/i686/asm/%.o: %.asm $(HEADERS_ASM) $(SOURCE_DIR)/src/arch/i686/isrs_gen.inc
	@mkdir -p $(@D)
	@$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<
	@echo "--> Compiled: " $<

$(SOURCE_DIR)/src/arch/i686/isrs_gen.c $(SOURCE_DIR)/src/arch/i686/isrs_gen.inc:
	@$(SOURCE_DIR)/build_scripts/generate_isrs.sh isrs_gen.c isrs_gen.inc

clean:
	rm -f $(BUILD_DIR)/arch/i686.a