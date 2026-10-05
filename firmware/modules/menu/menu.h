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

/* ---------- 界面编号 ----------
 * 前四个子界面必须紧跟 MENU_SCREEN_MAIN，顺序与主菜单项一致
 * （主菜单第 i 项按 3 进入 MENU_SCREEN_MAIN + 1 + i）。
 * 新增界面追加在后面，不要插在中间，否则会打乱这个对应关系。 */
typedef enum {
    MENU_SCREEN_MAIN = 0,   /* 主菜单 */
    MENU_SCREEN_LED,        /* LED 控制   —— 对应赛题第 2 项 */
    MENU_SCREEN_INFO,       /* 信息显示   —— 对应赛题第 3.a 项（纯显示页）*/
    MENU_SCREEN_TRACK,      /* 巡线功能   —— 对应赛题第 4 项 */
    MENU_SCREEN_EXT,        /* 拓展功能   —— 对应赛题第 6 项 */
    MENU_SCREEN_SPEED,      /* 速度设置   —— 巡线功能下的二级子菜单 */
    MENU_SCREEN_COUNT       /* 只用于计数，不是真实界面 */
} menu_screen_t;

/* ---------- OLED 显示规格 ----------
 * 常见 0.96"/1.3" 128x64 OLED 用 8x16 字体时：每行 16 个字符，共 8 行。
 * 如果你的屏幕或字体不同，改这两个宏即可。 */
#define MENU_LINE_CHARS  16
#define MENU_LINES_MAX   8

/* 主菜单项数量（赛题要求至少四项） */
#define MENU_MAIN_ITEMS  4

/* 每个子菜单最多几项（menu_set_status() 按这个上限检查行号） */
#define MENU_ITEM_MAX    4

/* ---------- 菜单项类型 ----------
 * MENU_ITEM_NONE 表示「这一项的功能不属于菜单模块」：箭头照样会指到它，
 * 但按键 3 落在这一项时菜单模块不做任何事，留给功能模块处理
 * （例如 LED 控制界面里的 LED1 开关，见 README.md 的主循环示例）。 */
typedef enum {
    MENU_ITEM_NONE = 0,
    MENU_ITEM_SUBMENU,      /* 按键 3 进入下一级子菜单 */
    MENU_ITEM_BACK          /* 按键 3 返回上一级 */
} menu_item_kind_t;

/* ---------- 接口函数 ---------- */

/* 上电初始化：界面设为主菜单，所有界面的箭头都指向首行。 */
void menu_init(void);

/* 每收到一次按键调用一次（硬件层负责消抖）。
 * 按键 1 / 2：当前界面里箭头上下移动（到边界回绕）；
 * 按键 3：进入子菜单 / 返回上一级 / 交给功能模块；
 * 按键 4：返回上一级（主菜单里无动作）。 */
void menu_on_key(menu_key_t key);

/* 当前所在界面。 */
menu_screen_t menu_screen(void);

/* 当前界面里箭头指向第几行，从 0 开始。
 * 功能模块靠它判断「用户选了哪一项」，例如 LED 界面里 0 = LED1、1 = LED2。 */
int menu_cursor(void);

/* 当前界面有几项；返回 0 表示这是纯显示页（例如 Information），
 * 页面上画什么完全由功能模块决定。 */
int menu_item_count(void);

/* 把「当前界面应该显示的内容」写进 lines，返回实际行数。
 * 硬件层拿到后逐行画到 OLED 即可，它不需要知道任何菜单逻辑。
 * 每个字符串最多 MENU_LINE_CHARS 个字符，调用方按返回的行数绘制。
 * 主菜单：只列菜单项（正好一屏 4 行）；子菜单：第 1 行是标题，后面是菜单项。 */
int menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines);

/* 给某个界面的某一行加一段右对齐的状态文字（例如 "ON" / "OFF" / "RUN"），
 * 传 NULL 清除。text 必须是静态存储（字面量或 static 数组），
 * 菜单模块只保存指针，不做拷贝。 */
void menu_set_status(menu_screen_t screen, int row, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* MENU_H */
