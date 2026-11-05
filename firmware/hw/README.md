# hw —— STM32F103C8T6 板端工程（寄存器裸机）

仓库里的板端工程：**不依赖 HAL / PlatformIO / CMSIS**，直接用寄存器，
用 `arm-none-eabi-gcc` + 手写 `linker.ld` + `Makefile` 构建。
对应赛题第 1、2 项的硬件侧；第 3 项串口、第 4 项巡线以后也接在这里。

> 赛题没有指定开发板和技术路线（答疑：只有推荐）；这里用 F103C8T6 是自选，
> 寄存器裸机也能完成赛题，HAL / PlatformIO 只是备选。

> **当前状态（2026-11-05，第 45 周）：底层验证阶段。**
> E1（点亮 PA5）、E2（空循环闪灯）、E3（读 PB1）已经做过并在板上验证；
> 当前 `main.c` 保留的是 E3 的“按住 PB1 点亮 PA5”（E1 / E3 见提交 `40ef7b8`）。
> E4（按键消抖）、E5（SysTick 1ms）正在写，`hw.c` 的接口定型后，
> 再把 OLED 和菜单接进来。
>
> 完整菜单固件此前在仓库外的试验工程里跑通过（英文 / 中文两版）；
> 仓库内这份按“底层自己写、显示层复用现成模板”的方式重做，
> 所以现在还不能直接烧出菜单。

## 接线

| 功能 | 引脚 | 说明 |
|---|---|---|
| OLED SCL / SDA | **PB11 / PB10** | 软件 I2C（GPIO 翻转，不占用硬件 I2C 外设） |
| OLED VCC / GND | 3V3 / GND | SSD1306 128×64，地址 `0x3C`（初始化时也会探 `0x3D`） |
| 按键 1（上移） | **PB1** → 另一端 GND | 内部上拉，按下读到 0 |
| 按键 2（下移） | **PB0** → 另一端 GND | 同上 |
| 按键 3（进入 / 执行） | **PA7** → 另一端 GND | 同上 |
| 按键 4（返回） | **PA6** → 另一端 GND | 同上 |
| LED 1 | **PA5** → 330Ω → 红色 LED → GND | **高电平点亮** |
| LED 2 | **PA4** → 330Ω → 红色 LED → GND | **高电平点亮** |
| 板载 LED | **PC13** | 低电平点亮；只作“屏幕没应答”的故障闪灯 |
| 串口 | 未接 | USB-TTL 已到；固件里还没有 USART 代码，等第 3 项 |

> 现在用的是“屏幕 + 按键一体”的模块。为了配合模块接线，I2C 先用 **GPIO 软件模拟**
> （`PB11 = SCL`、`PB10 = SDA`），方便自己调时序；芯片自带的 I2C 外设还没用上。
>
> 没有按键模块时，**用一根杜邦线把 PB1（或 PB0 / PA7 / PA6）碰一下 GND** 就是按了一次。
> 供电只选一种：ST-Link 的 3.3V 或板子的 USB，不要同时接。

## 构建 / 烧录

```bash
cd firmware/hw

make             # 产物：led_test.elf / led_test.hex / led_test.bin
make flash       # st-flash write led_test.bin 0x08000000
```

工具链在仓库根目录的 `nix-shell` 里（`gcc-arm-embedded` + `stlink`）：

```bash
cd ~/git/Debug-Car
nix-shell --run 'make -C firmware/hw'
nix-shell --run 'make -C firmware/hw flash'
```

工具链不在 PATH 时也可以指定：

```bash
make TOOLCHAIN=/path/to/gcc-arm-none-eabi/bin/arm-none-eabi-
```

> **当前 `Makefile` 只编译三个文件**：`src/main.c`、`src/startup.c`、`src/hw.c`，
> 产物名是 `led_test`。`oled.c`、`gfx.c`、字库、`menu_board.c` 已经在 `src/` 里，
> 但**还没加进 `SRCS`**；`LANG ?= en` 也是试验版留下的，暂时没生效。
> 等菜单接入时再改 `SRCS` / `NAME`，并像试验版那样用 `LANG` 在英文 5x7 和中文 12px
> 字库之间二选一。

## 当前进度（2026-11-05）

| 实验 | 状态 | 做了什么 / 还差什么 |
|---|---|---|
| E1 点亮 PA5 | ✅ | `hw_init()` 开 GPIOA 时钟、`CRL` 把 PA5 配成推挽输出；`led1_set()` 用 `BSRR` / `BRR` 置高、拉低 |
| E2 空循环闪灯 | ✅ | 先验证“能按时间翻转”；后来 `main.c` 被 E3 的按键点灯版本替换 |
| E3 读 PB1 | ✅ | PB1 配成上拉输入，按下读 0；当前 `main.c` 就是这一版 |
| E4 按键消抖 | 进行中 | `hw.h` 已定 `keys_scan()`：每 5 ms 调一次，连续 3 次一致（约 15 ms）才认 |
| E5 SysTick 1ms | 进行中 | `startup.c` 的向量表已留 `SysTick_Handler`；`hw.c` 还要补 `SysTick_Handler()` / `delay_ms()` / `millis()`（补齐前链接会报 `undefined reference to 'SysTick_Handler'`） |
| 接 OLED | 待做 | `oled.c` / `gfx.c` / `font5x7.c` 已拷入 `src/`，`menu_board.c` 的渲染函数已写好，未加进构建 |
| 接菜单 | 待做 | `menu.c` / `action.c` 在 `../menu/`，要一起编译；**先按赛题改 LED 控制屏的 1/2/4 按键**，再接板端 |
| 第 2d 状态显示 | 待做 | 赛题要求 LED 控制子菜单 1 s 内显示两灯状态；`menu_board.c` 目前只画标题和菜单行 |

`hw.h` 里已经定好的接口：`hw_init()`、`delay_ms()`、`millis()`、`led1_set()`、
`led2_set()`、`keys_scan()`、`key1_pressed()`（PB1 的调试用单键读取）。

## 文件

| 文件 | 说明 |
|---|---|
| `Makefile` | `arm-none-eabi-gcc` 构建、`objcopy` 出 hex/bin、`st-flash` 烧录 |
| `linker.ld` | 64 KB Flash / 20 KB RAM；中断向量表放 Flash 最前面 |
| `src/startup.c` | 复位处理：把 `.data` 从 Flash 搬到 RAM、`.bss` 清零、跳 `main`；向量表 |
| `src/regs.h` | RCC / GPIO / I2C / SysTick 的寄存器定义（不依赖 CMSIS） |
| `src/hw.c` / `hw.h` | 时钟、SysTick 时基、延时、两颗 LED、按键扫描（**本人写**；E4 / E5 还在补） |
| `src/main.c` | 当前硬件验证入口：按住 PB1 点亮 PA5（**本人写**） |
| `src/oled.c` / `oled.h` | 软件 I2C（PB11 / PB10）+ SSD1306 初始化、整屏按页刷新 |
| `src/gfx.c` / `gfx.h` | 1 KB 显存 + 5x7 ASCII 文字（`gfx_text()` 等） |
| `src/gfx_zh.c` + `font12.c` / `font12.h` | 12px 中文点阵，接口与 `gfx.c` 完全相同，编译时二选一 |
| `src/font5x7.c` / `font5x7.h` | 英文 5x7 点阵 |
| `src/menu_board.c` | 板端 `menu_render()`：清屏、标题居中、分隔线、菜单行、`oled_flush()` |
| `../menu/menu.c`、`action.c` | 菜单逻辑与动作，板端与 PC 共用；还没接进来 |

## 代码来源

- `hw.c`、`main.c`：本人所写，按 E1～E5 逐个实验实现（E1～E3 已在板上验证）。
- `startup.c`、`linker.ld`、`regs.h`、`oled.c`、`gfx.c`、`gfx_zh.c`、
  `font5x7.c`、`font12.c`：复用的模板（AI 生成），本人已逐段读懂用途。
  根 `README.md` 的 `## AI` 有说明。
- `menu_board.c`：板端渲染层，接口和 `menu_pc.c` 一样，只换了绘制方式。

## 已知坑

- **时间不能数循环次数。** `menu_draw()` 整屏刷一次约 120～150 ms（1 KB 显存、软件 I2C），
  主循环一刷屏，按圈数累加的时间就会慢几十倍。时间基准必须是硬件：
  `SysTick` 1 ms 中断里只做 `s_ms++`，主循环用 `millis()` 的差值算实际过了多少毫秒。
- **SysTick 中断里不要调 `menu_draw()`、不要放长循环**，只做计数。
- **软件 I2C 的速率**由 `oled.c` 的 `I2C_DELAY` 决定（现在是 5）：太小会 NACK，
  太大刷得更慢；SSD1306 上限 400 kHz。
- **LED 极性 / 颜色**：PC13 低电平亮，PA4 / PA5 高电平亮，接线和改代码时别搞反。
  红 / 绿 / 黄 LED 的 Vf ≈ 2 V，3.3 V 串 330Ω 约 4 mA，正常；蓝 / 白 Vf ≈ 3.0～3.4 V，
  同样 330Ω 只剩零点几 mA，会很暗甚至不亮 —— 用红灯，或把蓝灯电阻降到 100Ω 以下。
- **启动与链接**：向量表必须在 Flash 最前面；`.data` 的初值在 Flash 里，
  由 `Reset_Handler` 搬到 RAM，少了这一步全局变量初值就是错的。
- **烧录权限**：ST-Link 插上后普通用户可能没权限，需要 udev 规则或临时放开设备权限；
  拔掉后恢复。

## 待办

- [ ] E4 / E5 收尾：`hw.c` 补齐 `keys_scan()`、`SysTick_Handler()`、`delay_ms()`、
      `millis()`、`led2_set()`，`hw_init()` 配齐 PA4 / PA5 / PC13 和四个按键。
- [ ] 把 `oled.c` / `gfx.c` / 字库 / `menu_board.c` 加进 `Makefile`，先点亮屏幕。
- [ ] 接 `../menu/menu.c` 与 `action.c`：**先按赛题把 LED 控制屏改成按 1 / 2 / 4
      直接控制**，再把 `action.c` 的 `printf` / `time(NULL)` 换成
      `led1_set()` / `led2_set()` / `millis()`，主循环把 `millis()` 的差值喂给 `menu_tick()`。
- [ ] 第 2d 项：板端 OLED 显示两灯实际状态（1 s 内），需要 `action.c` 提供状态读取接口。
- [ ] 第 3a 项：信息屏同屏显示姓名拼音 / 学号 / 变量 `a`（初值 0），
      学号别再留 `2023000000` 占位。
- [ ] 板端字库按中文 / 英文选对（`gfx.c` 对 `gfx_zh.c`），否则中文画不出来。
- [ ] 第 3 项：加最小 USART1 初始化 + 发送函数（`PA9=TX` / `PA10=RX`）。
