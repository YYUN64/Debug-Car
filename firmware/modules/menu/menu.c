/* ============================================================
 * menu.c —— OLED 多级菜单状态机（纯逻辑实现，不依赖任何硬件）
 *
 * 对应赛题：基础部分第 1 项「OLED 菜单」（6 分）
 *   1.a 主菜单（3 分）
 *       - 上电后显示主菜单             -> menu_init()
 *       - 默认箭头指向首行             -> menu_init() 里把 s_cursor[] 清零
 *       - 按键 1 / 2 改变箭头指向行     -> menu_on_key() -> move_cursor()
 *       - 至少四个菜单项               -> MAIN_ITEMS[]
 *   1.b 子菜单（3 分）
 *       - 按键 3 进入当前选中的子菜单   -> menu_on_key() -> enter_item()
 *       - 按键 4 返回主菜单            -> menu_on_key() -> goto_back()
 *
 * 设计原则：这个文件里没有任何一行代码碰硬件。它只做两件事：
 *   (1) 根据「当前界面 + 按了哪个键」更新内部状态；
 *   (2) 把当前界面要显示的内容生成为字符串数组。
 * 「点亮 OLED 某一行像素」由硬件层实现，见 README.md 的集成示例。
 * 因此同一份代码可以在电脑上用 gcc 编译测试，拿到板子直接复用。
 *
 * 界面结构（每一屏的菜单项都写在下面的表里，加菜单项只改表，不改逻辑）：
 *
 *     主菜单  Main Menu
 *       ├─ LED Control   子菜单：LED1 / LED2 / All Off / Back
 *       ├─ Information   纯显示页（内容由功能模块画）
 *       ├─ Line Track    子菜单：Start / Stop / Speed ->（二级）Low / Mid / High / Back
 *       └─ Extension     子菜单：Buzzer / Servo Test / LED Flow / Back
 * ============================================================ */

#include "menu.h"

#include <stdio.h>    /* snprintf */
#include <stddef.h>   /* NULL */
#include <string.h>   /* strlen */

/* ---------- 菜单项表 ----------
 * kind 决定按键 3 落在这一项时菜单模块做什么：
 *   MENU_ITEM_SUBMENU -> 进入 target 指定的界面
 *   MENU_ITEM_BACK    -> 返回上一级
 *   MENU_ITEM_NONE    -> 菜单模块不管，交给功能模块（见 README.md）
 * 每一项的文字长度不要超过 MENU_LINE_CHARS - 2（要留出箭头和空格）。 */
typedef struct {
    const char *label;
    uint8_t     kind;
    uint8_t     target;     /* kind == MENU_ITEM_SUBMENU 时的目标界面 */
} menu_item_t;

/* 主菜单：四项都要能进下一级（对应赛题「至少四个菜单项」）。
 * 为什么用英文？常见 0.96"/1.3" OLED 模块自带字库只有 ASCII 字符，
 * 要在 OLED 上显示中文必须自己取模做字库，工作量大。
 * 赛题要求「显示自己的姓名（拼音）」也印证了考核预期是 ASCII 显示。
 * 如果你的 OLED 库支持中文，把下面四行换成中文即可。 */
static const menu_item_t MAIN_ITEMS[MENU_MAIN_ITEMS] = {
    { "LED Control", MENU_ITEM_SUBMENU, MENU_SCREEN_LED   },   /* 0: LED 控制 */
    { "Information", MENU_ITEM_SUBMENU, MENU_SCREEN_INFO  },   /* 1: 信息显示 */
    { "Line Track",  MENU_ITEM_SUBMENU, MENU_SCREEN_TRACK },   /* 2: 巡线功能 */
    { "Extension",   MENU_ITEM_SUBMENU, MENU_SCREEN_EXT   }    /* 3: 拓展功能 */
};

static const menu_item_t LED_ITEMS[] = {
    { "LED1",    MENU_ITEM_NONE,   0 },   /* 由 LED 功能模块处理按键 3 */
    { "LED2",    MENU_ITEM_NONE,   0 },
    { "All Off", MENU_ITEM_NONE,   0 },
    { "Back",    MENU_ITEM_BACK,   0 }
};

static const menu_item_t TRACK_ITEMS[] = {
    { "Start", MENU_ITEM_NONE,    0 },
    { "Stop",  MENU_ITEM_NONE,    0 },
    { "Speed", MENU_ITEM_SUBMENU, MENU_SCREEN_SPEED },
    { "Back",  MENU_ITEM_BACK,    0 }
};

static const menu_item_t SPEED_ITEMS[] = {
    { "Low",  MENU_ITEM_NONE, 0 },        /* 由巡线功能模块处理按键 3 */
    { "Mid",  MENU_ITEM_NONE, 0 },
    { "High", MENU_ITEM_NONE, 0 },
    { "Back", MENU_ITEM_BACK, 0 }
};

static const menu_item_t EXT_ITEMS[] = {
    { "Buzzer",     MENU_ITEM_NONE, 0 },  /* 由拓展功能模块处理按键 3 */
    { "Servo Test", MENU_ITEM_NONE, 0 },
    { "LED Flow",   MENU_ITEM_NONE, 0 },
    { "Back",       MENU_ITEM_BACK, 0 }
};

/* 界面属性表：标题、菜单项表、上一级界面。
 * count 用 sizeof 算出来，加减菜单项时不用手工改数字，也不会写错。 */
#define ITEM_COUNT(a)  ((int)(sizeof(a) / sizeof((a)[0])))

typedef struct {
    const char        *title;
    const menu_item_t *items;    /* NULL = 纯显示页 */
    int                count;
    uint8_t            parent;   /* 按键 4 返回哪一个界面 */
} menu_screen_def_t;

static const menu_screen_def_t SCREENS[MENU_SCREEN_COUNT] = {
    /* 主菜单也要有 count，否则按键 1/2/3 不会动；它不显示标题行，
     * 好让四项正好占满一屏（对应赛题「至少四个菜单项」）。 */
    { "Main Menu",   MAIN_ITEMS,  ITEM_COUNT(MAIN_ITEMS),  MENU_SCREEN_MAIN  },
    { "LED Control", LED_ITEMS,   ITEM_COUNT(LED_ITEMS),   MENU_SCREEN_MAIN  },
    { "Information", NULL,        0,                       MENU_SCREEN_MAIN  },
    { "Line Track",  TRACK_ITEMS, ITEM_COUNT(TRACK_ITEMS), MENU_SCREEN_MAIN  },
    { "Extension",   EXT_ITEMS,   ITEM_COUNT(EXT_ITEMS),   MENU_SCREEN_MAIN  },
    { "Speed",       SPEED_ITEMS, ITEM_COUNT(SPEED_ITEMS), MENU_SCREEN_TRACK }
};

/* ---------- 内部状态 ----------
 * static 表示「只有本文件能直接访问」，外部只能通过 menu_screen() /
 * menu_cursor() 读取。这样别人改不动菜单内部状态，接口更安全。 */
static menu_screen_t s_screen;                              /* 当前所在界面 */
static int           s_cursor[MENU_SCREEN_COUNT];           /* 每个界面各自的箭头位置 */
static const char   *s_status[MENU_SCREEN_COUNT][MENU_ITEM_MAX];  /* 右侧状态文字 */

/* ---------- 内部函数 ---------- */

/* 在当前界面里移动箭头。step = -1 上移，+1 下移。
 *
 * 为什么写成 (cur + step + n) % n ？
 *   先加 n 再取模，是为了让「箭头在首行时上移」能够回绕到最后一行。
 *   如果直接写 (cur - 1) % n，在 C 语言里 -1 % 4 的结果是 -1
 *   而不是 3，cur 就变成负数了 —— 这是新手常见的坑。 */
static void move_cursor(int step)
{
    int n = SCREENS[s_screen].count;

    if (n <= 0) {
        return;         /* 纯显示页没有箭头可移 */
    }
    s_cursor[s_screen] = (s_cursor[s_screen] + step + n) % n;
}

/* 按键 4（或选中 Back 项）：返回上一级界面。
 * 因为每个界面都有自己独立的 s_cursor[]，退回去时箭头会自动停在
 * 原来那一行 —— 不需要额外保存现场。 */
static void goto_back(void)
{
    if (s_screen != MENU_SCREEN_MAIN) {
        s_screen = (menu_screen_t)SCREENS[s_screen].parent;
    }
}

/* 按键 3：进入子菜单 / 返回上一级；MENU_ITEM_NONE 的项留给功能模块。 */
static void enter_item(void)
{
    const menu_screen_def_t *def = &SCREENS[s_screen];
    const menu_item_t       *item;

    if (def->count <= 0) {
        return;
    }
    item = &def->items[s_cursor[s_screen]];

    if (item->kind == MENU_ITEM_SUBMENU) {
        s_screen = (menu_screen_t)item->target;
    } else if (item->kind == MENU_ITEM_BACK) {
        goto_back();
    }
    /* MENU_ITEM_NONE：什么都不做，功能模块会自己处理这一项 */
}

/* 生成一行：箭头 + 文字 + 右对齐的状态。 */
static void format_line(char *out, int selected, const char *label, const char *status)
{
    int status_len = (status != NULL) ? (int)strlen(status) : 0;
    int limit = MENU_LINE_CHARS - status_len;   /* 文字最多占到这一列（不含）*/
    int i;

    for (i = 0; i < MENU_LINE_CHARS; i++) {
        out[i] = ' ';
    }
    out[MENU_LINE_CHARS] = '\0';

    /* 箭头指向的行开头画 '>'，其它行用空格占位，保证文字左对齐 */
    out[0] = selected ? '>' : ' ';

    for (i = 0; label[i] != '\0' && (i + 1) < limit; i++) {
        out[i + 1] = label[i];
    }
    for (i = 0; i < status_len; i++) {
        out[MENU_LINE_CHARS - status_len + i] = status[i];
    }
}

/* ---------- 对外接口 ---------- */

void menu_init(void)
{
    int i;
    int j;

    s_screen = MENU_SCREEN_MAIN;    /* 上电显示主菜单 */
    for (i = 0; i < MENU_SCREEN_COUNT; i++) {
        s_cursor[i] = 0;            /* 默认箭头指向首行 */
        for (j = 0; j < MENU_ITEM_MAX; j++) {
            s_status[i][j] = NULL;
        }
    }
}

menu_screen_t menu_screen(void)
{
    return s_screen;
}

int menu_cursor(void)
{
    return s_cursor[s_screen];
}

int menu_item_count(void)
{
    return SCREENS[s_screen].count;
}

void menu_on_key(menu_key_t key)
{
    switch (key) {
    case MENU_KEY_1: move_cursor(-1); break;   /* 按键 1：箭头上一行 */
    case MENU_KEY_2: move_cursor(+1); break;   /* 按键 2：箭头下一行 */
    case MENU_KEY_3: enter_item();    break;   /* 按键 3：进入 / 返回 / 交给功能模块 */
    case MENU_KEY_4: goto_back();     break;   /* 按键 4：返回上一级 */
    default: break;                            /* MENU_KEY_NONE */
    }
}

void menu_set_status(menu_screen_t screen, int row, const char *text)
{
    if ((int)screen >= MENU_SCREEN_COUNT || row < 0 || row >= MENU_ITEM_MAX) {
        return;                     /* 越界调用直接忽略，不写坏内存 */
    }
    s_status[screen][row] = text;
}

int menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines)
{
    const menu_screen_def_t *def = &SCREENS[s_screen];
    int n = 0;
    int i;

    if (lines == NULL || max_lines <= 0) {
        return 0;
    }

    /* 子菜单第一行显示标题；主菜单不显示标题，好让四项占满一屏 */
    if (s_screen != MENU_SCREEN_MAIN && n < max_lines) {
        snprintf(lines[n], MENU_LINE_CHARS + 1, "%-*s", MENU_LINE_CHARS, def->title);
        n++;
    }

    if (def->count <= 0) {
        /* 纯显示页（Information）：页面内容由功能模块自己画，
         * 这里只给一行返回提示。 */
        if (n < max_lines) {
            snprintf(lines[n], MENU_LINE_CHARS + 1, "%-*s", MENU_LINE_CHARS, "4: Back");
            n++;
        }
        return n;
    }

    for (i = 0; i < def->count && n < max_lines; i++) {
        /* snprintf 的第二个参数是缓冲区总长度（含结尾的 '\0'），
         * 所以传 MENU_LINE_CHARS + 1 —— 这也是它能防止溢出的原因。 */
        char line[MENU_LINE_CHARS + 1];

        format_line(line, i == s_cursor[s_screen], def->items[i].label, s_status[s_screen][i]);
        snprintf(lines[n], MENU_LINE_CHARS + 1, "%s", line);
        n++;
    }

    return n;
}
