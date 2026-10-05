# menu —— OLED 多级菜单状态机

对应赛题 **基础部分第 1 项「OLED 菜单」（6 分）**。

## 这个模块做什么

只做菜单的**逻辑**，不碰任何硬件。赛题要求与实现的对应关系：

| 赛题要求 | 实现位置 |
|---|---|
| 上电后显示主菜单 | `menu_init()` 把界面设为 `MENU_SCREEN_MAIN` |
| 默认箭头指向首行 | `menu_init()` 里 `s_cursor = 0` |
| 按键 1、2 改变箭头指向行 | `menu_on_key()` → `main_menu_move()` |
| 主菜单至少四个菜单项 | `MAIN_ITEM_TEXT[]` + `MENU_MAIN_ITEMS` |
| 按键 3 进入当前选中的子菜单 | `menu_on_key()` → `main_menu_enter()` |
| 按键 4 返回主菜单 | `menu_on_key()` 的子菜单分支 |

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

## 接口

```c
void          menu_init(void);                 /* 上电调用一次 */
void          menu_on_key(menu_key_t key);     /* 每收到一次按键调用一次 */
menu_screen_t menu_screen(void);               /* 当前界面 */
int           menu_cursor(void);               /* 主菜单箭头位置，0 起 */
int           menu_render(char lines[][MENU_LINE_CHARS + 1], int max_lines);
```

`menu_render()` 把当前界面要显示的内容填进 `lines` 并返回实际行数。
硬件层拿到后逐行画到 OLED 即可 —— 它完全不需要知道菜单逻辑。

## 在电脑上测试

需要 `gcc` 和 `make`（在 NixOS 上进入仓库根目录的 `nix-shell` 就都有了）。

```bash
make run        # 编译并运行测试
make clean      # 清理
```

测试程序会模拟按下一系列按键，把每一屏画成方块打印出来，
并按赛题验收流程逐条断言。

## 怎么集成到单片机主循环

```c
menu_init();                        /* 上电初始化一次 */

for (;;) {
    menu_key_t k = key_scan();      /* 你自己的按键扫描（含消抖） */

    if (k != MENU_KEY_NONE) {
        menu_on_key(k);             /* 菜单处理 1/2 移动、3 进入、4 返回 */

        /* 子菜单里把按键转给对应的功能模块 */
        switch (menu_screen()) {
        case MENU_SCREEN_LED:   led_screen_on_key(k);   break;
        case MENU_SCREEN_TRACK: track_screen_on_key(k); break;
        default: break;
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

## 待办 / 扩展点

- [ ] 主菜单加第 5 项 `Settings`（参数设置），对应拓展部分第 6.2 项
- [ ] 子菜单显示具体状态（例如 LED 控制界面显示两个 LED 的当前亮灭）
- [ ] 如果 OLED 库支持中文，把 `MAIN_ITEM_TEXT[]` 换成中文
