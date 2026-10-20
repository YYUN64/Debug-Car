# 在电脑上编译并运行 menu 模块
#
#   make run    编译 + 运行
#   make clean  清理
#
# 编译产物放在 build/ 目录（已在仓库 .gitignore 中忽略）。

CC         ?= gcc
CFLAGS     ?= -std=c99 -Wall -Wextra -O0 -g

BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/menu

# 要编译的 .c 文件。以后再多几个就继续往后面加
SRCS       := menu.c action.c

$(TARGET): $(SRCS) action.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: run clean
