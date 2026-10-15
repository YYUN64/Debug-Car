# 2026 DEBUG 实验室软件组考核 · 参赛工程

循迹小车的完整工程代码。**基础部分 60 分 + 拓展部分 40 分。**

---

## 赛题给分项对照

| 项 | 内容 | 分值 | 计划代码位置 | 状态 |
|---|---|---|---|---|
| 1 | OLED 多级菜单 | 6 | `firmware/menu/` | 进行中（菜单逻辑已跑通，绘制已与逻辑分离，待接硬件） |
| 2 | LED 控制 | 6 | `firmware/modules/led/` | 未开始 |
| 3 | 人机交互与串口通信 | 8 | `firmware/modules/protocol/`、`host/` | 未开始 |
| 4 | 巡线（传感器 / 一圈 / 两圈 / 特殊元素 / 完整地图） | 35 | `firmware/modules/track/` | 未开始 |
| 5 | Git 仓库与开发记录 | 5 | 本仓库 | 进行中 |
| 6.1 | 随机地图循迹 | 15 | `firmware/modules/track/` | 未开始 |
| 6.2 | 参数设置与掉电保存 | 5 | `firmware/modules/config/` | 未开始 |
| 6.3 | 遥控小车 | 10 | `host/` | 未开始 |
| 6.4 | 巡线速度与控制性能 | 10 | `firmware/modules/track/pid/` | 未开始 |

基础部分必须拿到完整 60 分，才能参加拓展部分（40 分）的线下考核。
给分代码集中在 `firmware/` 下：菜单框架在 `firmware/menu/`，各功能模块在 `firmware/modules/`。

## 目录结构

```
.
├── README.md
├── .gitignore
├── shell.nix          PC 端开发环境：gcc / make / python3(pyserial)
└── firmware/          小车固件工程（主控板 STM32F103，Arm Cortex-M3）
    ├── menu/          OLED 多级菜单 —— 整个固件的顶层框架
    │                  build() 生成这一屏的文字，draw() 负责送到屏幕
    └── modules/        各功能模块：LED / 串口 / 巡线 / 参数设置…
```

> `firmware/modules/` 目前还是空的 —— Git 不跟踪空目录，等第一个功能模块落地时它自然会出现。

## 关键约束（决定了整个工程架构）

基础部分第 1～4 项要求**一镜到底**录制：从第 1 项连续录到第 4 项，中途不得停止录制，
也不得剪辑、暂停、加速，且**录制开始后不得修改代码、重新编译或重新烧录**。

这条规则直接决定了架构：

- **必须一份固件覆盖全部基础项**，不能按功能拆成多个工程；
- **OLED 菜单是整个固件的顶层框架**，所有功能都从菜单进入；
- **主循环不允许阻塞延时**（不用 `delay`），一律「定时器中断 + 状态机」。
  否则 LED 交替闪烁（要求 30 秒 × 3 轮不停闪）、OLED 状态刷新（要求 1 秒内更新）、
  按键响应会互相干扰，一定会挂。

## 模块移植约定

目标平台已确定为 **STM32F103 + STM32 HAL 库**（详见「开发环境」）。
`firmware/modules/` 下的模块遵守两条约定，目标是「先在电脑上写和测，拿到板子直接移植」：

1. **纯 C**，不使用单片机厂商私有类型，统一用 `<stdint.h>` 的 `uint8_t` 等标准类型；
2. 硬件操作通过**回调函数指针注入**（例如「点亮 OLED 某段」「读某个按键」），
   模块自身不直接碰寄存器，也不直接调用 HAL。

这样每个模块都能配一个 PC 上的测试程序，用 gcc 编译运行、验证逻辑，
不必等硬件到位。`firmware/menu/` 已经按这个思路分好了层：
`build()` 只生成要显示的文字，`draw()` 才是碰屏幕的那一层。

## 开发环境

**全部开发工作都在 Linux（NixOS）上完成**，工具链由仓库根目录的 `shell.nix` 提供，
`nix-shell` 一条命令进入，不需要装任何全局软件。

| 项目 | 内容 |
|---|---|
| 主控板 | **STM32F103 系列**（Arm Cortex-M3 内核，72 MHz） |
| 开发方式 | **STM32 HAL 库** |
| 固件框架 | PlatformIO + `framework = stm32cube`（即 ST 官方 STM32Cube HAL） |
| 交叉编译器 | `arm-none-eabi-gcc` |
| 烧录 / 调试 | ST-Link（`openocd` / `st-flash`）；无 ST-Link 时可用串口 ISP（`stm32flash`） |
| 串口终端 | `picocom` 或 `pio device monitor` |
| 上位机 | Python 3 + pyserial |
| 版本管理 | Git + GitHub |

> 培训教程（`环境配置教程.pdf`）走的是 **Windows + Keil MDK + STM32CubeMX** 路线。
> Keil MDK 只有 Windows 版，Linux 上用不了；但赛题只要求 **STM32F103 + HAL 库**，
> IDE 是自选的，因此这里改用 PlatformIO。教程中仍然有效的部分是
> **芯片型号、HAL 库、以及"烧录后板载 LED 闪烁即链路正常"这条验收标准**。

### 工作流

```bash
cd ~/git/Debug-Car
nix-shell                         # 进入开发环境（arm-none-eabi-gcc / openocd / stlink / picocom / platformio）

pio boards STM32F103              # 列出支持的 F103 板子
pio run                           # 编译
pio run -t upload                 # 烧录（ST-Link）
pio device monitor                # 看串口输出
```

**板子 id 待定** —— 培训板的具体型号要等拿到板子才知道，对应关系参考：

| 培训板 | PlatformIO board id |
|---|---|
| BluePill / 最小系统板 STM32F103C8T6（64K Flash） | `bluepill_f103c8` |
| 同上，标称 128K Flash | `bluepill_f103c8_128k` |
| BlackPill STM32F103C8T6 | `blackpill_f103c8` |
| ST Nucleo-F103RB | `nucleo_f103rb` |

> 板子到了以后拿芯片丝印去 `pio boards STM32F103` 里对；不能确定时先用
> `bluepill_f103c8`（F103C8T6 是最常见的培训芯片）。

### 烧录方式

| 手头有什么 | 怎么烧 |
|---|---|
| ST-Link（教程里那种） | `pio run -t upload`，或 `st-flash write .pio/build/<board>/firmware.bin 0x8000000` |
| 只有 USB 转串口 | F103 自带 ROM 引导程序：BOOT0 接 1 再上电，用 `stm32flash` 走 USART1 烧录 |

**验收标准（与培训教程一致）**：编译烧录后**板载 LED 闪烁**，说明
「工具链 + HAL + 烧录」整条链路已通。能找到芯片型号并不等于工程一定能编译成功。

### 常见问题

- **PlatformIO 的工具链报 `No such file or directory`**：NixOS 上预编译二进制通常找不到
  `/lib64/ld-linux-x86-64.so.2`。解法是在系统配置里开 `programs.nix-ld.enable = true;`。
- **串口 `Permission denied`**：用户不在 `dialout` 组，或 udev 规则未安装。
- **ST-Link 认不到设备**：`services.udev.packages` 里没加 `openocd` / `stlink`，
  或需要重新插拔一次。

> **环境待验证（2026-10-15）**：`pio --version`（6.1.19）与 `pio boards STM32F103`
> （列出 20 块 F103 板子）已实测可用；但**端到端编译还没跑通** ——
> 首次下载 arm-none-eabi 工具链时卡住了。等板子到手第一次 `pio run` 时验证，
> 若报上面第一条错误，就按上面的办法开 `nix-ld`。

## 开发计划

| 阶段 | 内容 | 对应给分项 | 状态 |
|---|---|---|---|
| 0 | C 语言基础、Git 工作流、工具链（Linux + PlatformIO + `arm-none-eabi-gcc`） | 第 5 项 | 进行中 |
| 1 | OLED 多级菜单状态机 | 第 1 项 | 进行中 |
| 2 | LED 控制与状态显示 | 第 2 项 | 未开始 |
| 3 | 串口协议与上位机 | 第 3 项 | 未开始 |
| 4 | 电机驱动、灰度传感器、基础巡线 | 第 4.a / 4.b 项 | 未开始 |
| 5 | PID 调参、特殊元素、完整地图 | 第 4.c / 4.d / 4.e 项 | 未开始 |
| 6 | 拓展部分 | 第 6 项 | 未开始 |

## 提交规范

赛题第 5 项要求「每周至少产生一次有效提交」，且只修改提交时间、提交说明或与赛题无关的
内容不计为有效提交。

- 提交说明格式：`类型: 一句话说清做了什么`
  - `feat` 新增功能 · `fix` 修 bug · `docs` 文档 · `chore` 杂项 · `refactor` 重构
- **每周至少一次**与给分项目相关的真实代码提交；
- 不提交编译产物（见 `.gitignore`）；
- 提交作者姓名与邮箱对应本人，邮箱已绑定 GitHub 账号。

## 说明

- 本仓库代码由本人独立完成，不与其他选手共用。
- 赛题原文要点与评分细则整理在各模块的 `README.md` 里。
