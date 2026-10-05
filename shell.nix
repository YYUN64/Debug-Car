{ pkgs ? import <nixpkgs> { } }:

# 本仓库的开发环境。在仓库根目录执行 `nix-shell` 进入。
#
# 已提供：
#   gcc / gnumake   —— 在电脑上编译并运行 firmware/modules 下的模块测试
#   python3         —— 上位机（赛题第 3 项、6.3），已带 pyserial
#
# 单片机工具链等培训确定主控板型号后再启用（见下方注释）。
# 如果培训指定的工具链是 Keil，那是 Windows 专有软件，NixOS 上跑不了，
# 需要改用 arm-none-eabi-gcc + openocd 的方案，或另找一台 Windows 烧录。

pkgs.mkShell {
  packages = with pkgs; [
    gcc
    gnumake
    (python3.withPackages (ps: [ ps.pyserial ]))

    # ---- 拿到主控板后按需取消注释 ----
    # gcc-arm-embedded    # arm-none-eabi-gcc，STM32 交叉编译
    # openocd             # 下载 / 调试
    # stlink              # ST-Link 工具
    # picocom             # 串口调试
  ];
}
