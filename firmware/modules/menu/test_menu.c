/* ============================================================
 * test_menu.c —— 在电脑上验证 menu 模块的逻辑
 *
 * 编译运行（需要 gcc 和 make）：
 *     make run
 * 或者手动：
 *     gcc -std=c99 -Wall -Wextra menu.c test_menu.c -o test_menu && ./test_menu
 *
 * 它按赛题第 1 项的验收流程走一遍，把每一屏画成方块打印出来，
 * 并逐条断言。退出码：全部通过为 0，有失败为 1。
 * ============================================================ */

#include <stdio.h>

#include "menu.h"

static int s_fail = 0;   /* 失败的断言数量 */

/* 断言：打印结果，失败则计数 */
static void check(const char *what, int ok)
{
    printf("    [%s] %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) {
        s_fail++;
    }
}

/* 把 menu_render() 的输出画成一个方块，模拟 OLED 的一屏 */
static void draw(void)
{
    char lines[MENU_LINES_MAX][MENU_LINE_CHARS + 1];
    int  n = menu_render(lines, MENU_LINES_MAX);

    printf("    +------------------+\n");
    for (int i = 0; i < n; i++) {
        printf("    |%-16s|\n", lines[i]);
    }
    printf("    +------------------+\n");
}

/* 模拟按下一次按键（硬件层在真机上做的事） */
static void press(menu_key_t k, const char *name)
{
    printf("  >> 按下 %s\n", name);
    menu_on_key(k);
    draw();
}

int main(void)
{
    printf("=== 测试 1：上电后显示主菜单，箭头指向首行 ===\n");
    menu_init();
    draw();
    check("当前界面是主菜单", menu_screen() == MENU_SCREEN_MAIN);
    check("箭头在首行", menu_cursor() == 0);

    printf("\n=== 测试 2：按键 2 向下移动箭头 ===\n");
    menu_init();
    press(MENU_KEY_2, "按键2");
    check("箭头移到第 1 行", menu_cursor() == 1);
    press(MENU_KEY_2, "按键2");
    check("箭头移到第 2 行", menu_cursor() == 2);

    printf("\n=== 测试 3：按键 1 向上移动箭头 ===\n");
    press(MENU_KEY_1, "按键1");
    check("箭头回到第 1 行", menu_cursor() == 1);

    printf("\n=== 测试 4：在首行按按键 1，应回绕到最后一行 ===\n");
    menu_init();
    press(MENU_KEY_1, "按键1");
    check("箭头回绕到最后一行", menu_cursor() == MENU_MAIN_ITEMS - 1);

    printf("\n=== 测试 5：按键 3 进入各个子菜单，按键 4 返回 ===\n");
    menu_init();
    for (int i = 0; i < MENU_MAIN_ITEMS; i++) {
        printf("  ---- 第 %d 项 ----\n", i);
        press(MENU_KEY_3, "按键3 进入");
        check("已进入对应子菜单",
              menu_screen() == (menu_screen_t)(MENU_SCREEN_MAIN + 1 + i));
        press(MENU_KEY_4, "按键4 返回");
        check("已返回主菜单", menu_screen() == MENU_SCREEN_MAIN);
        if (i + 1 < MENU_MAIN_ITEMS) {
            press(MENU_KEY_2, "按键2 下移");
        }
    }

    printf("\n=== 测试 6：主菜单里按按键 4 不应有任何影响 ===\n");
    menu_init();
    press(MENU_KEY_4, "按键4");
    check("仍在主菜单", menu_screen() == MENU_SCREEN_MAIN);
    check("箭头仍在首行", menu_cursor() == 0);

    printf("\n================ 结果 ================\n");
    if (s_fail == 0) {
        printf("全部断言通过\n");
    } else {
        printf("有 %d 项失败\n", s_fail);
    }
    return s_fail ? 1 : 0;
}
