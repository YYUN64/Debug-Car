/**
  ******************************************************************************
  * @file    oled.h
  * @brief   SSD1306 (128x64, I2C) 底层：初始化与整屏刷新
  ******************************************************************************
  */

#ifndef OLED_H
#define OLED_H

#include <stdint.h>

/**
  * @brief 初始化 OLED，自动探测 0x3C / 0x3D
  * @retval 探测到的 7 位地址；0 表示没有应答
  */
uint8_t oled_init(void);

/** @brief 把 gfx 的 1KB 显存按页刷到屏幕 */
void oled_flush(void);

#endif /* OLED_H */
