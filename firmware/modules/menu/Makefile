# 在电脑上编译并运行 menu 模块的测试
#
#   make run    编译 + 运行
#   make clean  清理
#
# 编译产物放在 build/ 目录（已在仓库 .gitignore 中忽略）。

CC         ?= gcc
CFLAGS     ?= -std=c99 -Wall -Wextra -O0 -g

BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/test_menu

$(TARGET): menu.c test_menu.c menu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) menu.c test_menu.c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: run clean
