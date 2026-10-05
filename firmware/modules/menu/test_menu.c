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
#include <string.h>

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

/* ---------- 给下面的断言用的小工具 ---------- */

/* 渲染当前界面，把行数写进 *n_out */
static int render(char lines[][MENU_LINE_CHARS + 1], int *n_out)
{
    int n = menu_render(lines, MENU_LINES_MAX);

    if (n_out != NULL) {
        *n_out = n;
    }
    return n;
}

/* 渲染结果里 '>' 箭头在第几行（没有返回 -1） */
static int arrow_row(void)
{
    char lines[MENU_LINES_MAX][MENU_LINE_CHARS + 1];
    int  n = render(lines, NULL);

    for (int i = 0; i < n; i++) {
        if (lines[i][0] == '>') {
            return i;
        }
    }
    return -1;
}

/* 这一行除了箭头之外还有没有文字 */
static int has_text(const char *line)
{
    for (int i = 1; i < MENU_LINE_CHARS; i++) {
        if (line[i] != ' ') {
            return 1;
        }
    }
    return 0;
}

/* line 是否以 suffix 结尾（用来验证状态文字右对齐） */
static int ends_with(const char *line, const char *suffix)
{
    size_t ll = strlen(line);
    size_t sl = strlen(suffix);

    if (sl > ll) {
        return 0;
    }
    return strcmp(line + (ll - sl), suffix) == 0;
}

int main(void)
{
    char lines[MENU_LINES_MAX][MENU_LINE_CHARS + 1];
    int  n;

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

    printf("\n=== 测试 7：主菜单渲染出四项，箭头只在一行上 ===\n");
    menu_init();
    n = render(lines, NULL);
    check("渲染出 4 行", n == MENU_MAIN_ITEMS);
    check("箭头在第 1 行", lines[0][0] == '>');
    check("其它行没有箭头", lines[1][0] != '>' && lines[2][0] != '>' && lines[3][0] != '>');
    check("四行都有菜单项文字", has_text(lines[0]) && has_text(lines[1]) &&
                                has_text(lines[2]) && has_text(lines[3]));

    printf("\n=== 测试 8：子菜单里箭头也能上下移动 ===\n");
    menu_init();
    press(MENU_KEY_3, "按键3 进入 LED 控制");
    check("菜单记录当前界面有几项", menu_item_count() == 4);
    check("进入后箭头在第 1 项", menu_cursor() == 0);
    press(MENU_KEY_2, "按键2");
    check("箭头移到第 2 项", menu_cursor() == 1);
    check("渲染出来箭头在第 3 行（第 1 行是标题）", arrow_row() == 2);
    press(MENU_KEY_1, "按键1");
    check("箭头移回第 1 项", menu_cursor() == 0);

    printf("\n=== 测试 9：二级子菜单（巡线功能 -> 速度设置）===\n");
    menu_init();
    press(MENU_KEY_2, "按键2");
    press(MENU_KEY_2, "按键2");
    check("主菜单箭头在 Line Track", menu_cursor() == 2);
    press(MENU_KEY_3, "按键3 进入巡线功能");
    check("进入 Line Track", menu_screen() == MENU_SCREEN_TRACK);
    press(MENU_KEY_2, "按键2");
    press(MENU_KEY_2, "按键2");
    check("箭头停在 Speed 项", menu_cursor() == 2);
    press(MENU_KEY_3, "按键3 进入速度设置");
    check("进入二级子菜单 Speed", menu_screen() == MENU_SCREEN_SPEED);
    press(MENU_KEY_4, "按键4 返回");
    check("回到 Line Track", menu_screen() == MENU_SCREEN_TRACK);
    check("Line Track 的箭头仍停在 Speed 那一项", menu_cursor() == 2);
    press(MENU_KEY_4, "按键4 再返回");
    check("回到主菜单", menu_screen() == MENU_SCREEN_MAIN);
    check("主菜单箭头仍停在 Line Track", menu_cursor() == 2);

    printf("\n=== 测试 10：选中 Back 项按按键 3 也能返回 ===\n");
    menu_init();
    press(MENU_KEY_3, "按键3 进入 LED 控制");
    press(MENU_KEY_2, "按键2");
    press(MENU_KEY_2, "按键2");
    press(MENU_KEY_2, "按键2");
    check("箭头在最后一项 Back", menu_cursor() == 3);
    press(MENU_KEY_3, "按键3 选中 Back");
    check("已返回主菜单", menu_screen() == MENU_SCREEN_MAIN);

    printf("\n=== 测试 11：功能模块可以给菜单项加右侧状态文字 ===\n");
    menu_init();
    menu_set_status(MENU_SCREEN_LED, 0, "ON");
    menu_set_status(MENU_SCREEN_LED, 1, "OFF");
    press(MENU_KEY_3, "按键3 进入 LED 控制");
    render(lines, &n);
    check("LED1 那一行右侧显示 ON", n >= 2 && ends_with(lines[1], "ON"));
    check("LED2 那一行右侧显示 OFF", n >= 3 && ends_with(lines[2], "OFF"));
    check("状态文字不影响箭头所在行", lines[1][0] == '>');

    printf("\n=== 测试 12：信息显示是纯显示页（没有菜单项）===\n");
    menu_init();
    press(MENU_KEY_2, "按键2");
    press(MENU_KEY_3, "按键3 进入信息显示");
    check("进入 Information", menu_screen() == MENU_SCREEN_INFO);
    check("菜单模块不提供菜单项，内容交给功能模块", menu_item_count() == 0);
    press(MENU_KEY_1, "按键1");
    press(MENU_KEY_2, "按键2");
    check("纯显示页里按 1 / 2 不会改变界面", menu_screen() == MENU_SCREEN_INFO);
    press(MENU_KEY_4, "按键4 返回");
    check("返回主菜单", menu_screen() == MENU_SCREEN_MAIN);

    printf("\n================ 结果 ================\n");
    if (s_fail == 0) {
        printf("全部断言通过\n");
    } else {
        printf("有 %d 项失败\n", s_fail);
    }
    return s_fail ? 1 : 0;
}
