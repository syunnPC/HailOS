# ディレクトリ設定
SRC_DIR := Kernel
LIB_DIR := Library
OUT_DIR := build

# 共通 C ファイル一覧（interrupt.c は除外）
C_SOURCES := \
    $(SRC_DIR)/hailkrnl.c \
    $(SRC_DIR)/init.c \
    $(LIB_DIR)/MemoryManager/memmgr.c \
    $(LIB_DIR)/Utility/util.c \
    $(LIB_DIR)/Vga/vga.c \
    $(LIB_DIR)/Vga/print.c

# 割り込みハンドラ用 C ファイル（個別ビルドルールで扱う）
INTERRUPT_SRC := $(SRC_DIR)/interrupt.c

# 出力ファイル
OUTPUT := $(OUT_DIR)/hailos.elf

# コンパイラとリンカ
CC := gcc
LD := ld

# 共通コンパイルフラグ（SSE, MMX 無効、ベクトル最適化禁止）
CFLAGS := -ffreestanding -O2 -Wall -Wextra -nostdlib \
          -mno-sse -mno-sse2 -mno-mmx -fno-tree-vectorize \
          -I$(SRC_DIR) -I$(LIB_DIR)

# リンカオプション
LDFLAGS := -Ttext 0x100000 -e main -m elf_x86_64

# オブジェクトファイル一覧（interrupt.c の .o を手動追加）
OBJ := $(patsubst %.c, $(OUT_DIR)/%.o, $(C_SOURCES)) \
       $(OUT_DIR)/Kernel/interrupt.o

# 全体ビルドターゲット
all: $(OUTPUT)

# ELF 出力
$(OUTPUT): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

# 一般的な .c → .o ルール（共通ソース）
$(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 割り込みハンドラは FPU 命令も禁止
$(OUT_DIR)/Kernel/interrupt.o: $(SRC_DIR)/interrupt.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -mno-80387 -c $< -o $@

# クリーンターゲット
clean:
	rm -rf $(OUT_DIR)

.PHONY: all clean