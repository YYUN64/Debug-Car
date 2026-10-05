/* ============================================================
 * menu.h —— OLED 多级菜单状态机（对外接口）
 *
 * 对应赛题：基础部分第 1 项「OLED 菜单」（6 分）
 *
 * 使用方式见同目录 README.md。
 * 本模块是纯逻辑，不含任何硬件代码，可以在电脑上直接编译测试。
 * ============================================================ */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- 按键编号 ----------
 * 赛题里的「按键 1 / 2 / 3 / 4」，由硬件层扫描后传进来。
 * MENU_KEY_NONE 表示这一轮没有按键按下。 */
typedef enum {
    MENU_KEY_NONE = 0,
    MENU_KEY_1,
    MENU_KEY_2,
    MENU_KEY_3,
    MENU_KEY_4
} menu_key_t;

/* ---------- 界面编号 ---------- */
typedef enum {
    MENU_SCREEN_MAIN = 0,   /* 主菜单 */
    MENU_SCREEN_LED,        /* LED 控制   —— 对应赛题第 2 项 */
    MENU_SCREEN_INFO,       /* 信息显示   —— 对应赛题第 3.a 项 */
    MENU_SCREEN_TRACK,      /* 巡线功能   —— 对应赛题第 4 项 */
    MENU_SCREEN_EXT,        /* 拓展功能   —— 对应赛题第 6 项 */
    MENU_SCREEN_COUNT       /* 只用于计数，不是真实界面 */
} menu_screen_t;

/* ---------- OLED 显示规格 ----------
 * 常见 0.96"/1.3" 128x64 OLED 用 8x16 字体时：每行 16 个字符，共 8 行。
 * 如果你的屏幕或字体不同，改这两个宏即可。 */
#define MENU_LINE_CHARS  16
#define MENU_LINES_MAX   8

/* 主菜单项数量（赛题要求至少四项） */
#define MENU_MAIN_ITEMS  4

/* ---------- 接口函数 ---------- */

/* 上电初始化：界面设为主菜单，箭头指向首行。 */
void menu_init(void);

/* 每收到一次按键调用一次（硬件层负责消抖）。 */
void menu_on_key(menu_key_t key);

/* 当前所在界面。 */
menu_screen_t menu_screen(void);

/* 主菜单箭头指向的行号，从 0 开始。 */
int menu_cursor(void);

/* 把「当前界面应该显示的内容」写进 lines，返回实际行数。
 * 硬件层拿到后逐行画到 OLED 即可，它不需要知道任何菜单逻辑。
 * 每个字符串最多 MENU_LINE_CHARS 个字符，调用方按返回的行数绘制。 */
int menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines);

#ifdef __cplusplus
}
#endif

#endif /* MENU_H */
