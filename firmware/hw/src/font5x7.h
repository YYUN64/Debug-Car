/**
  ******************************************************************************
  * @file    font5x7.h
  * @brief   5x7 ASCII 字库声明
  ******************************************************************************
  */
#ifndef FONT5X7_H
#define FONT5X7_H

#include <stdint.h>

/* font5x7[c - 0x20][col]：字符 c 第 col 列的点阵，bit0 是最上面一行 */
extern const uint8_t font5x7[96][5];

#endif /* FONT5X7_H */
