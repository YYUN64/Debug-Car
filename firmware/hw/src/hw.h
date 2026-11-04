/**
  ******************************************************************************
  * @file    hw.h
  * @brief   最底层硬件：延时/毫秒计数、板载 LED、4 个按键
  *
  * 按键接法（内部上拉，按下接地）：
  *   PB1 = 按键 1（上移）    PB0 = 按键 2（下移）
  *   PA7 = 按键 3（进入）    PA6 = 按键 4（返回）
  ******************************************************************************
  */

#ifndef HW_H
#define HW_H

#include <stdint.h>

void hw_init(void);

/** @brief 忙等延时（SysTick，1ms 精度），同时推进 millis() */
void delay_ms(uint32_t ms);

/** @brief 开机以来的毫秒数（由 delay_ms 推进） */
uint32_t millis(void);

/** @brief LED1（PA5，高电平点亮） */
void led1_set(int on);

/** @brief LED2（PA4，高电平点亮） */
void led2_set(int on);

/** @brief 扫描按键，返回 1~4；没有新按下返回 0（内部已消抖，每 5ms 调一次） */
int keys_scan(void);

/** PB1，按下返回 1 */
int key1_pressed(void);

#endif /* HW_H */
