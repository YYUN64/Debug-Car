# 需要 arm-none-eabi-gcc（STM32CubeIDE 自带，或 apt/nix 安装）
# 如果工具链不在 PATH 里，可以这样指定：
#   make TOOLCHAIN=/opt/gcc-arm-none-eabi/bin/arm-none-eabi-
#   make            英文版（5x7 ASCII，约 4.4KB）  -> menu_fw.bin

TOOLCHAIN ?= arm-none-eabi-
CC      := $(TOOLCHAIN)gcc
OBJCOPY := $(TOOLCHAIN)objcopy
SIZE    := $(TOOLCHAIN)size

LANG ?= en

SRCS := src/main.c src/startup.c src/hw.c
NAME := led_test

CFLAGS  := -mcpu=cortex-m3 -mthumb -O2 -g3 -Wall -Wextra -ffreestanding \
           -fno-builtin -std=gnu11 -Isrc
LDFLAGS := -T linker.ld -nostdlib -Wl,--gc-sections \
           -Wl,-Map=build/$(NAME).map -Wl,--print-memory-usage

OBJS   := $(patsubst src/%.c,build/%.o,$(SRCS))
TARGET := $(NAME).elf

all: $(TARGET) $(NAME).hex $(NAME).bin

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) linker.ld | build
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@
	$(SIZE) $@

$(NAME).hex: $(TARGET)
	$(OBJCOPY) -O ihex $< $@

$(NAME).bin: $(TARGET)
	$(OBJCOPY) -O binary $< $@

# 需要先装 stlink 工具（st-flash）
flash: $(NAME).bin
	st-flash write $(NAME).bin 0x08000000

clean:
	rm -rf build menu_fw.elf menu_fw.hex menu_fw.bin \
	       menu_fw_zh.elf menu_fw_zh.hex menu_fw_zh.bin

.PHONY: all clean flash
