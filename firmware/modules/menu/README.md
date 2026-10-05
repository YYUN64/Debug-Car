# menu —— OLED 多级菜单状态机

对应赛题 **基础部分第 1 项「OLED 菜单」（6 分）**。

## 这个模块做什么

只做菜单的**逻辑**，不碰任何硬件。赛题要求与实现的对应关系：

| 赛题要求 | 实现位置 |
|---|---|
| 上电后显示主菜单 | `menu_init()` 里 `s_screen = MENU_SCREEN_MAIN` |
| 默认箭头指向首行 | `menu_init()` 把 `s_cursor[]` 全部清零 |
| 按键 1、2 改变箭头指向行 | `menu_on_key()` → `move_cursor()`（到边界回绕） |
| 主菜单至少四个菜单项 | `MAIN_ITEMS[]`，共 `MENU_MAIN_ITEMS` 项 |
| 按键 3 进入当前选中的子菜单 | `menu_on_key()` → `enter_item()` |
| 按键 4 返回主菜单 | `menu_on_key()` → `goto_back()`，按 `SCREENS[].parent` 逐级返回 |

## 界面结构

```
主菜单（一屏 4 行，不显示标题，正好放下四项）
 ├─ LED Control   子菜单：LED1 / LED2 / All Off / Back
 ├─ Information   纯显示页：内容由功能模块自己画，菜单模块只给一行 "4: Back"
 ├─ Line Track    子菜单：Start / Stop / Speed →（二级）Low / Mid / High / Back
 └─ Extension     子菜单：Buzzer / Servo Test / LED Flow / Back
```

每一屏的菜单项都写在 `menu.c` 顶部的表里：

- `MAIN_ITEMS[]`、`LED_ITEMS[]`、`TRACK_ITEMS[]`、`SPEED_ITEMS[]`、`EXT_ITEMS[]`
  —— 每一项是 `{ 文字, 类型, 目标界面 }`；
- `SCREENS[]` —— 每个界面的标题、菜单项表、上一级界面（`count` 用 `sizeof` 自动算）。

菜单项类型只有三种：

| 类型 | 按键 3 落在这一项时 |
|---|---|
| `MENU_ITEM_SUBMENU` | 菜单模块自己跳进 `target` 界面（例如 Speed → 二级子菜单） |
| `MENU_ITEM_BACK` | 返回上一级（和按键 4 等价） |
| `MENU_ITEM_NONE` | 菜单模块**什么都不做**，留给功能模块处理（例如 LED1 开关） |

## 为什么这么设计

赛题规定基础部分第 1～4 项**一镜到底**录制，中途不得停止、剪辑，
且**录制开始后不得修改代码、重新编译、重新烧录**。这意味着：

- **必须一份固件覆盖全部基础项**，不能按功能拆成多个工程；
- **OLED 菜单就是整个固件的顶层框架**，所有功能都从菜单进入。

同时为了让模块能「先在电脑上写和测，拿到板子直接移植」，这里做了严格分层：

```
   硬件层（你的 OLED 驱动 + 按键扫描）
          │  menu_on_key(按键)        ↑  menu_render(要显示的文字)
          ↓                           │
       menu.c —— 本模块，纯 C，不含任何硬件代码
```

两个实现细节值得说明：

- **每个界面有自己独立的箭头位置**（`s_cursor[MENU_SCREEN_COUNT]`）。
  从二级子菜单退回巡线功能时，箭头会自动停在原来那一项，不需要额外压栈保存现场。
- **状态文字用指针保存**（`menu_set_status()`）。菜单模块不拷贝字符串，
  所以功能模块传进来的必须是静态存储（字面量或 `static` 数组），不要传局部数组。

## 接口

```c
void          menu_init(void);                 /* 上电调用一次 */
void          menu_on_key(menu_key_t key);     /* 每收到一次按键调用一次 */
menu_screen_t menu_screen(void);               /* 当前界面 */
int           menu_cursor(void);               /* 当前界面里箭头指向第几行，0 起 */
int           menu_item_count(void);           /* 当前界面有几项，0 = 纯显示页 */
int           menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines);
void          menu_set_status(menu_screen_t screen, int row, const char *text);
```

`menu_render()` 把当前界面要显示的内容填进 `lines` 并返回实际行数：

- 主菜单：只列菜单项（一屏 4 行，不占标题行）；
- 子菜单：第 1 行是标题，后面是菜单项，箭头行开头是 `'>'`；
- 纯显示页：只给一行 `4: Back`，页面主体由功能模块绘制。

硬件层拿到后逐行画到 OLED 即可 —— 它完全不需要知道菜单逻辑。

## 加菜单项 / 加一级子菜单

1. 往对应的 `xxx_ITEMS[]` 里加一行（文字长度别超过 `MENU_LINE_CHARS - 2`）；
2. 新界面要在 `menu.h` 的 `menu_screen_t` **末尾**加一个 `MENU_SCREEN_xxx`
   （插在中间会打乱「主菜单第 i 项 = `MENU_SCREEN_MAIN + 1 + i`」的对应关系），
   然后在 `SCREENS[]` 里加一行、填好 `parent`，并在父菜单里加一个 `SUBMENU` 项。

`MENU_ITEM_MAX`（默认 4）只限制「能加状态文字的行数」，菜单项本身可以更多
（一屏最多显示 `MENU_LINES_MAX` 行，扣掉标题行）。

## 在电脑上测试

需要 `gcc` 和 `make`（在 NixOS 上进入仓库根目录的 `nix-shell` 就都有了）。

```bash
make run        # 编译并运行测试
make clean      # 清理
```

测试共 12 组、42 条断言：主菜单四项、按键 1/2 移动与回绕、按键 3 进入各级子菜单、
按键 4 逐级返回且箭头位置保留、二级子菜单、Back 项、状态文字右对齐、纯显示页。
每一屏都会画成方块打印出来，方便肉眼对照赛题验收流程。

## 怎么集成到单片机主循环

```c
menu_init();                        /* 上电初始化一次 */

for (;;) {
    menu_key_t k = key_scan();      /* 你自己的按键扫描（含消抖） */

    if (k != MENU_KEY_NONE) {
        menu_screen_t before = menu_screen();

        menu_on_key(k);             /* 菜单处理 1/2 移动、3 进入、4 返回 */

        /* 子菜单里把按键转给对应的功能模块（菜单模块不处理 MENU_ITEM_NONE 的项）。
         * 用 menu_cursor() 判断用户选中了哪一项，用 menu_set_status() 把状态显示在右侧。 */
        if (before == menu_screen()) {
            switch (menu_screen()) {
            case MENU_SCREEN_LED:   led_screen_on_key(k, menu_cursor());   break;
            case MENU_SCREEN_TRACK: track_screen_on_key(k, menu_cursor()); break;
            case MENU_SCREEN_SPEED: track_speed_on_key(k, menu_cursor());  break;
            default: break;
            }
        }
    }

    char lines[MENU_LINES_MAX][MENU_LINE_CHARS + 1];
    int  n = menu_render(lines, MENU_LINES_MAX);
    oled_draw_lines(lines, n);      /* 你自己的 OLED 绘制函数 */
}
```

**注意**：主循环里不要用阻塞延时（`delay`）。OLED 刷新和按键扫描都应由定时器驱动，
否则第 2 项要求的「LED 交替闪烁 30 秒 × 3 轮不停闪」和「状态改变后 1 秒内更新 OLED」
会互相干扰，一定会挂。

`before == menu_screen()` 这个判断是为了区分「菜单自己切换了界面」和
「按键留在当前界面、该交给功能模块」：如果菜单已经跳转（例如刚进入子菜单），
就不要把这次按键再喂给功能模块，否则会出现「按一下 3 又立刻触发一次功能」。

## 待办 / 扩展点

- [x] 子菜单支持多项、支持箭头选择（本版本）
- [x] 子菜单显示具体状态（`menu_set_status()`，例如 LED 界面显示 ON / OFF）
- [x] 二级子菜单（巡线功能 → 速度设置）
- [ ] 主菜单加第 5 项 `Settings`（参数设置），对应拓展部分第 6.2 项
- [ ] 接上 LED / 巡线 / 拓展功能模块，把 `MENU_ITEM_NONE` 的项填上真实动作
- [ ] 如果 OLED 库支持中文，把各个 `xxx_ITEMS[]` 的文字换成中文
