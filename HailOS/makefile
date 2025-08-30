# HailOS Makefile

# --- Toolchain Configuration ---
# Adjust if your cross-compiler has a different prefix
TARGET_ARCH = x86_64
TARGET_TRIPLET = $(TARGET_ARCH)-elf
CC = $(TARGET_TRIPLET)-gcc
AS = $(TARGET_TRIPLET)-as # Generally CC can be used for .S files too
LD = $(TARGET_TRIPLET)-ld
OBJCOPY = $(TARGET_TRIPLET)-objcopy

# --- Build Directories ---
BUILD_DIR = Build
OBJ_DIR = $(BUILD_DIR)/obj
KERNEL_ELF = $(BUILD_DIR)/kernel.elf # Changed name for clarity
# KERNEL_BIN = $(BUILD_DIR)/hailos_kernel.bin # If you need a raw binary later

# --- Source Directories ---
# Note: These paths are relative to the Makefile's location.
KERNEL_SRC_DIR = Kernel
KERNEL_LIB_SRC_DIR = KernelLibrary

LIBRARY_DIR = Library
LIB_COMMON_SRC_DIR = $(LIBRARY_DIR)/Common
LIB_CONSOLE_SRC_DIR = $(LIBRARY_DIR)/Console
LIB_FILESYSTEM_DIR = $(LIBRARY_DIR)/Filesystem
LIB_FS_FAT32_SRC_DIR = $(LIB_FILESYSTEM_DIR)/Fat32

LIB_IO_DIR = $(LIBRARY_DIR)/Io
LIB_IO_COMMON_SRC_DIR = $(LIB_IO_DIR)/Common
LIB_IO_DISK_DIR = $(LIB_IO_DIR)/Disk
LIB_IO_DISK_ATA_SRC_DIR = $(LIB_IO_DISK_DIR)/ATA
LIB_IO_DISK_AHCI_SRC_DIR = $(LIB_IO_DISK_DIR)/AHCI
LIB_IO_PIC_SRC_DIR = $(LIB_IO_DIR)/Pic
LIB_IO_PS2_SRC_DIR = $(LIB_IO_DIR)/Ps2
LIB_IO_TSC_SRC_DIR = $(LIB_IO_DIR)/Tsc
LIB_IO_PCI_SRC_DIR = $(LIB_IO_DIR)/PCI

LIB_MEMMGR_SRC_DIR = $(LIBRARY_DIR)/MemoryManager
LIB_TIMER_SRC_DIR = $(LIBRARY_DIR)/Timer
LIB_UTIL_SRC_DIR = $(LIBRARY_DIR)/Utility
LIB_UTIL_FILE_SRC_DIR = $(LIB_UTIL_SRC_DIR)/File
LIB_UTIL_BMP_SRC_DIR = $(LIB_UTIL_SRC_DIR)/BitmapImage
LIB_VGA_SRC_DIR = $(LIBRARY_DIR)/Vga
LIB_HAL_DIR = $(LIBRARY_DIR)/HAL
LIB_HAL_DISK_DIR = $(LIB_HAL_DIR)/Disk

UI_DIR = UserInterface
STDC_SRC_DIR = Stdc

# --- Compiler and Linker Flags ---
# -g: Add debug symbols
# -ffreestanding: No standard library, no hosted environment
# -Wall -Wextra: Enable many warnings
# -mno-red-zone: For kernel code, avoid red zone optimization
# -mcmodel=kernel: For kernel code, allows symbols to be >2GB apart
# -fno-pie: Don't produce Position Independent Executables
# -fno-stack-protector: Disable stack smashing protector
CFLAGS_COMMON = -g -ffreestanding -Wall -Wextra -mno-red-zone -mcmodel=kernel -fno-pie -fno-stack-protector -O0
CFLAGS = $(CFLAGS_COMMON)
ASFLAGS = -g # For assembly, -g for debug symbols.

# Specific flags for interrupt.c (adjust if needed, e.g. for FPU state saving)
CFLAGS_INTERRUPT = $(CFLAGS_COMMON) -mgeneral-regs-only # Example: avoid using MMX/SSE/AVX registers in ISR stubs

# Include paths for header files
INCLUDES = \
    -I$(KERNEL_SRC_DIR) \
    -I$(KERNEL_LIB_SRC_DIR) \
    -I$(LIB_COMMON_SRC_DIR) \
    -I$(LIB_CONSOLE_SRC_DIR) \
    -I$(LIB_FS_FAT32_SRC_DIR) \
    -I$(LIB_IO_COMMON_SRC_DIR) \
    -I$(LIB_IO_DISK_ATA_SRC_DIR) \
    -I$(LIB_IO_PIC_SRC_DIR) \
    -I$(LIB_IO_PS2_SRC_DIR) \
    -I$(LIB_IO_TSC_SRC_DIR) \
    -I$(LIB_MEMMGR_SRC_DIR) \
    -I$(LIB_TIMER_SRC_DIR) \
    -I$(LIB_UTIL_SRC_DIR) \
    -I$(LIB_VGA_SRC_DIR) \
    -I$(STDC_SRC_DIR) \
    -I$(LIB_UTIL_FILE_SRC_DIR) \
    -I$(LIB_UTIL_BMP_SRC_DIR) \
    -I$(LIB_IO_PCI_SRC_DIR) \
    -I$(LIB_IO_DISK_AHCI_SRC_DIR) \
    -I$(LIB_HAL_DISK_DIR) \
    -I$(UI_DIR)

# Linker script
LDSCRIPT = linker.ld
LDFLAGS = -T $(LDSCRIPT) -nostdlib -n # -n makes text segment read-only

# --- Source Files ---
# Assembly source files (e.g., boot.S must be first for ENTRY(_start) if it defines _start)
# Assuming boot.S is in KernelLibrary and is the entry point
ASM_SOURCES = \
    $(KERNEL_LIB_SRC_DIR)/boot.S # Adjust if boot.S is elsewhere or not used

# C source files
# List all .c files with their full paths from the project root (where Makefile is)
C_SOURCES_KERNEL = \
    $(KERNEL_SRC_DIR)/hailkrnl.c

C_SOURCES_KERNEL_LIB = \
    $(KERNEL_LIB_SRC_DIR)/init.c \
    # $(KERNEL_LIB_SRC_DIR)/interrupt.c # Handled separately due to special flags

C_SOURCES_LIBRARY = \
    $(LIB_CONSOLE_SRC_DIR)/system_console.c \
    $(LIB_FS_FAT32_SRC_DIR)/fat32.c \
    $(LIB_IO_COMMON_SRC_DIR)/io.c \
    $(LIB_IO_DISK_ATA_SRC_DIR)/ata.c \
    $(LIB_IO_PIC_SRC_DIR)/pic.c \
    $(LIB_IO_PS2_SRC_DIR)/ps2kbd.c \
    $(LIB_IO_TSC_SRC_DIR)/tsc.c \
    $(LIB_MEMMGR_SRC_DIR)/memmgr.c \
    $(LIB_MEMMGR_SRC_DIR)/memutil.c \
    $(LIB_TIMER_SRC_DIR)/timer.c \
    $(LIB_UTIL_SRC_DIR)/util.c \
    $(LIB_VGA_SRC_DIR)/vga.c \
    $(KERNEL_SRC_DIR)/krnl_init.c \
    $(LIB_IO_PS2_SRC_DIR)/ps2kbdapi.c \
    $(LIB_IO_PS2_SRC_DIR)/keycode.c \
    $(LIB_UTIL_FILE_SRC_DIR)/file.c \
    $(LIB_UTIL_BMP_SRC_DIR)/bitmap.c \
    $(LIB_IO_PCI_SRC_DIR)/pci.c \
    $(LIB_IO_DISK_AHCI_SRC_DIR)/ahci.c \
    $(LIB_HAL_DISK_DIR)/hal_disk.c \
    $(LIB_IO_PS2_SRC_DIR)/ps2mouse.c

C_SOURCES_STDC = \
    $(STDC_SRC_DIR)/string.c

C_SOURCES_UI = \
    $(UI_DIR)/cursor.c

# All C sources except the one with special flags
C_SOURCES = \
    $(C_SOURCES_KERNEL) \
    $(C_SOURCES_KERNEL_LIB) \
    $(C_SOURCES_LIBRARY) \
    $(C_SOURCES_STDC) \
    $(C_SOURCES_UI)

# Special C source file for interrupts (if it needs different flags)
C_SOURCE_INTERRUPT = $(KERNEL_LIB_SRC_DIR)/interrupt.c

# --- Object Files ---
# For each source file path like "Dir/SubDir/file.c", its object will be "Build/obj/Dir/SubDir/file.o"
OBJ_FILES_ASM = $(patsubst %.S,$(OBJ_DIR)/%.o,$(ASM_SOURCES))
OBJ_FILES_C = $(patsubst %.c,$(OBJ_DIR)/%.o,$(filter-out $(C_SOURCE_INTERRUPT),$(C_SOURCES)))
OBJ_FILE_INTERRUPT = $(patsubst %.c,$(OBJ_DIR)/%.o,$(C_SOURCE_INTERRUPT))

ALL_OBJS = $(OBJ_FILES_ASM) $(OBJ_FILES_C) $(OBJ_FILE_INTERRUPT)

# --- Build Rules ---
all: $(KERNEL_ELF)

$(KERNEL_ELF): $(ALL_OBJS) $(LDSCRIPT)
	@echo "LD   $@"
	@mkdir -p $(@D) # Ensure Build directory exists
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(ALL_OBJS) -lgcc # Link with libgcc for compiler intrinsics

# Generic rule for compiling C files
# $< is the first prerequisite (the .c file)
# $@ is the target (the .o file)
# $(@D) is the directory part of the target
$(OBJ_DIR)/%.o: %.c
	@echo "CC   $<"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Specific rule for interrupt.c (if it needs special flags)
$(OBJ_FILE_INTERRUPT): $(C_SOURCE_INTERRUPT)
	@echo "CC-INT $<"
	@mkdir -p $(@D)
	$(CC) $(CFLAGS_INTERRUPT) $(INCLUDES) -c $< -o $@

# Generic rule for assembling .S files
$(OBJ_DIR)/%.o: %.S
	@echo "AS   $<"
	@mkdir -p $(@D)
	$(CC) $(ASFLAGS) $(INCLUDES) -c $< -o $@ # Using CC to assemble .S files is common

# Optional: Create a raw binary (e.g., for some bootloaders or emulators)
# $(KERNEL_BIN): $(KERNEL_ELF)
# 	@echo "OBJCOPY $@"
# 	$(OBJCOPY) -O binary $< $@

# --- Phony Targets ---
.PHONY: all clean qemu

clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR)

# Example QEMU rule (adjust as needed)
qemu: $(KERNEL_ELF)
	qemu-system-x86_64 -kernel $(KERNEL_ELF) -serial stdio

# Debug: print variables
print-%:
	@echo '$* = $($*)'