# HailOS Makefile

# --- Toolchain Configuration ---
# Adjust if your cross-compiler has a different prefix
TARGET_ARCH = x86_64
TARGET_TRIPLET = $(TARGET_ARCH)-elf
CC = $(TARGET_TRIPLET)-gcc
AS = $(TARGET_TRIPLET)-as
LD = $(TARGET_TRIPLET)-ld
OBJCOPY = $(TARGET_TRIPLET)-objcopy

# --- Build Directories ---
BUILD_DIR = Build
OBJ_DIR = $(BUILD_DIR)/obj
KERNEL_ELF = $(BUILD_DIR)/kernel.elf
# KERNEL_BIN = $(BUILD_DIR)/kernel.bin # If you need a raw binary later

# --- Source Directories ---
KERNEL_SRC_DIR = Kernel
LIB_COMMON_SRC_DIR = Library/Common
LIB_MEMMGR_SRC_DIR = Library/MemoryManager
LIB_UTIL_SRC_DIR = Library/Utility
LIB_VGA_SRC_DIR = Library/Vga
LIB_DISKIO_SRC_DIR = Library/DiskIo
LIB_IOMGR_SRC_DIR = Library/IoManager
LIB_STDC_SRC_DIR = Library/Stdc

# --- Compiler and Linker Flags ---
# -g: Add debug symbols
# -ffreestanding: No standard library, no hosted environment
# -Wall -Wextra: Enable many warnings
# -mno-red-zone: For kernel code, avoid red zone optimization
# -mcmodel=kernel: For kernel code, allows symbols to be >2GB apart
# -fno-pie: Don't produce Position Independent Executables (unless you design for it)
CFLAGS_COMMON = -g -ffreestanding -Wall -Wextra -mno-red-zone -mcmodel=kernel -fno-pie
CFLAGS = $(CFLAGS_COMMON)
ASFLAGS = -g # For 64-bit assembly, -g for debug symbols is sufficient.

# Specific flags for interrupt.c
CFLAGS_INTERRUPT = $(CFLAGS_COMMON) -mno-80387 -mno-sse -mno-sse2 -mno-mmx -mno-3dnow -mno-avx

# Include paths
INCLUDES = -I$(KERNEL_SRC_DIR) \
           -I$(LIB_COMMON_SRC_DIR) \
           -I$(LIB_MEMMGR_SRC_DIR) \
           -I$(LIB_UTIL_SRC_DIR) \
           -I$(LIB_VGA_SRC_DIR) \
		   -I$(LIB_DISKIO_SRC_DIR) \
		   -I$(LIB_IOMGR_SRC_DIR) \
		   -I$(LIB_STDC_SRC_DIR)

# Linker script
LDSCRIPT = linker.ld
LDFLAGS = -T $(LDSCRIPT) -nostdlib -n # -n makes text segment read-only (good practice)

# --- Source Files ---
# Assembly source files (boot.S must be first for ENTRY(_start))
ASM_SOURCES = $(KERNEL_SRC_DIR)/boot.S

# C source files
C_SOURCES = \
    $(KERNEL_SRC_DIR)/hailkrnl.c \
    $(KERNEL_SRC_DIR)/init.c \
    $(LIB_MEMMGR_SRC_DIR)/memmgr.c \
    $(LIB_UTIL_SRC_DIR)/util.c \
    $(LIB_VGA_SRC_DIR)/print.c \
    $(LIB_VGA_SRC_DIR)/vga.c \
	$(LIB_VGA_SRC_DIR)/bitmap.c \
	$(LIB_STDC_SRC_DIR)/string.c \
	$(LIB_STDC_SRC_DIR)/assert.c \
	$(LIB_IOMGR_SRC_DIR)/iomgr.c \
	$(LIB_IOMGR_SRC_DIR)/timeinfo.c \
	$(LIB_DISKIO_SRC_DIR)/ata.c \
	$(LIB_DISKIO_SRC_DIR)/fat32.c \
	$(LIB_DISKIO_SRC_DIR)/fileio.c \
	$(LIB_STDC_SRC_DIR)/stdio.c

# Special C source file for interrupts
C_SOURCE_INTERRUPT = $(KERNEL_SRC_DIR)/interrupt.c

# Object files
OBJ_FILES_ASM = $(patsubst $(KERNEL_SRC_DIR)/%.S,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
OBJ_FILES_C = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter-out $(C_SOURCE_INTERRUPT),$(C_SOURCES)))
OBJ_FILE_INTERRUPT = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCE_INTERRUPT))

ALL_OBJS = $(OBJ_FILES_ASM) $(OBJ_FILES_C) $(OBJ_FILE_INTERRUPT)

# --- Build Rules ---
all: $(KERNEL_ELF)

$(KERNEL_ELF): $(ALL_OBJS) $(LDSCRIPT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) -lgcc
	@echo "LD   $@"

# Rule for general C files
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
	@echo "CC   $@"

# Specific rule for interrupt.c
$(OBJ_FILE_INTERRUPT): $(C_SOURCE_INTERRUPT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_INTERRUPT) $(INCLUDES) -c $< -o $@
	@echo "CC-INT $@"

# Rule for assembly files
$(OBJ_DIR)/%.o: $(KERNEL_SRC_DIR)/%.S
	@mkdir -p $(@D)
	# Use $(CC) to invoke the assembler. It will pass its default target (x86_64)
	# and can handle C preprocessor directives if you use .S (capital S)
	$(CC) $(ASFLAGS) $(INCLUDES) -c $< -o $@
	@echo "AS   $@"

# Optional: Create a raw binary (e.g., for some bootloaders or emulators)
# $(KERNEL_BIN): $(KERNEL_ELF)
# 	$(OBJCOPY) -O binary $< $@
# 	@echo "OBJCOPY $@"

# --- phony targets ---
.PHONY: all clean

clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)

# Debug: print variables
print-%:
	@echo '$*=$($*)'