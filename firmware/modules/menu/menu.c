/* ============================================================
 * menu.c —— OLED 多级菜单状态机（纯逻辑实现，不依赖任何硬件）
 *
 * 对应赛题：基础部分第 1 项「OLED 菜单」（6 分）
 *   1.a 主菜单（3 分）
 *       - 上电后显示主菜单             -> menu_init()
 *       - 默认箭头指向首行             -> menu_init() 里 s_cursor = 0
 *       - 按键 1 / 2 改变箭头指向行     -> menu_on_key() -> main_menu_move()
 *       - 至少四个菜单项               -> MAIN_ITEM_TEXT[]
 *   1.b 子菜单（3 分）
 *       - 按键 3 进入当前选中的子菜单   -> main_menu_enter()
 *       - 按键 4 返回主菜单            -> menu_on_key() 的 else 分支
 *
 * 设计原则：这个文件里没有任何一行代码碰硬件。它只做两件事：
 *   (1) 根据「当前界面 + 按了哪个键」更新内部状态；
 *   (2) 把当前界面要显示的内容生成为字符串数组。
 * 「点亮 OLED 某一行像素」由硬件层实现，见 README.md 的集成示例。
 * 因此同一份代码可以在电脑上用 gcc 编译测试，拿到板子直接复用。
 * ============================================================ */

#include "menu.h"

#include <stdio.h>    /* snprintf */
#include <stddef.h>   /* NULL */

/* ---------- 主菜单项文字 ----------
 * 为什么用英文？常见 0.96"/1.3" OLED 模块自带字库只有 ASCII 字符，
 * 要在 OLED 上显示中文必须自己取模做字库，工作量大。
 * 赛题要求「显示自己的姓名（拼音）」也印证了考核预期是 ASCII 显示。
 * 如果你的 OLED 库支持中文，把下面四行换成中文即可。 */
static const char *const MAIN_ITEM_TEXT[MENU_MAIN_ITEMS] = {
    "LED Control",     /* 0: LED 控制 */
    "Information",     /* 1: 信息显示 */
    "Line Track",      /* 2: 巡线功能 */
    "Extension"        /* 3: 拓展功能 */
};

/* ---------- 内部状态 ----------
 * static 表示「只有本文件能直接访问」，外部只能通过 menu_screen() /
 * menu_cursor() 读取。这样别人改不动菜单内部状态，接口更安全。 */
static menu_screen_t s_screen;   /* 当前所在界面 */
static int           s_cursor;   /* 主菜单箭头指向第几行，从 0 开始 */

/* ---------- 内部函数 ---------- */

/* 子菜单第一行显示的标题。 */
static const char *screen_title(menu_screen_t s)
{
    switch (s) {
    case MENU_SCREEN_LED:   return "LED Control";
    case MENU_SCREEN_INFO:  return "Information";
    case MENU_SCREEN_TRACK: return "Line Track";
    case MENU_SCREEN_EXT:   return "Extension";
    default:                return "Main Menu";
    }
}

/* 在主菜单里移动箭头。step = -1 上移，+1 下移。
 *
 * 为什么写成 (s_cursor + step + N) % N ？
 *   先加 N 再取模，是为了让「箭头在首行时上移」能够回绕到最后一行。
 *   如果直接写 (s_cursor - 1) % N，在 C 语言里 -1 % 4 的结果是 -1
 *   而不是 3，s_cursor 就变成负数了 —— 这是新手常见的坑。 */
static void main_menu_move(int step)
{
    s_cursor = (s_cursor + step + MENU_MAIN_ITEMS) % MENU_MAIN_ITEMS;
}

/* 在主菜单里按按键 3：进入箭头指向的子菜单。 */
static void main_menu_enter(void)
{
    switch (s_cursor) {
    case 0: s_screen = MENU_SCREEN_LED;   break;
    case 1: s_screen = MENU_SCREEN_INFO;  break;
    case 2: s_screen = MENU_SCREEN_TRACK; break;
    case 3: s_screen = MENU_SCREEN_EXT;   break;
    default: break;   /* 理论上到不了这里 */
    }
}

/* ---------- 对外接口 ---------- */

void menu_init(void)
{
    s_screen = MENU_SCREEN_MAIN;   /* 上电显示主菜单 */
    s_cursor = 0;                  /* 默认箭头指向首行 */
}

menu_screen_t menu_screen(void)
{
    return s_screen;
}

int menu_cursor(void)
{
    return s_cursor;
}

void menu_on_key(menu_key_t key)
{
    if (s_screen == MENU_SCREEN_MAIN) {
        /* ---------- 当前在主菜单 ---------- */
        switch (key) {
        case MENU_KEY_1: main_menu_move(-1); break;   /* 按键 1：箭头上一行 */
        case MENU_KEY_2: main_menu_move(+1); break;   /* 按键 2：箭头下一行 */
        case MENU_KEY_3: main_menu_enter();  break;   /* 按键 3：进入子菜单 */
        default: break;                               /* 按键 4 在主菜单无定义 */
        }
    } else {
        /* ---------- 当前在某个子菜单 ---------- */
        if (key == MENU_KEY_4) {
            s_screen = MENU_SCREEN_MAIN;   /* 按键 4：返回主菜单 */
        }
        /* 子菜单内部的其它按键（例如 LED 控制界面里的按键 1 / 2）
         * 由对应功能模块处理，不在这里写 —— 见 README.md 的主循环示例。 */
    }
}

int menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines)
{
    int n = 0;

    if (lines == NULL || max_lines <= 0) {
        return 0;
    }

    if (s_screen == MENU_SCREEN_MAIN) {
        for (int i = 0; i < MENU_MAIN_ITEMS && n < max_lines; i++) {
            /* 箭头指向的行开头画 '>'，其它行用空格占位，保证文字左对齐。
             * snprintf 的第二个参数是缓冲区总长度（含结尾的 '\0'），
             * 所以传 MENU_LINE_CHARS + 1 —— 这也是它能防止溢出的原因。 */
            snprintf(lines[n], MENU_LINE_CHARS + 1, "%c%s",
                     (i == s_cursor) ? '>' : ' ', MAIN_ITEM_TEXT[i]);
            n++;
        }
    } else {
        /* 子菜单：第一行显示标题，第二行提示怎么返回 */
        snprintf(lines[n], MENU_LINE_CHARS + 1, "%s", screen_title(s_screen));
        n++;
        if (n < max_lines) {
            snprintf(lines[n], MENU_LINE_CHARS + 1, "4: Back");
            n++;
        }
    }

    return n;
}
