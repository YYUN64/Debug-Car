/**
  * @file    font12.h
  * @brief   12px 点阵字库声明（中文版菜单用）
  */
#ifndef FONT12_H
#define FONT12_H

#include <stdint.h>

#define FONT_ASCII_FIRST  32
#define FONT_ASCII_COUNT  95
#define FONT_CJK_COUNT    62

extern const uint16_t font_cjk_cp[FONT_CJK_COUNT];
extern const uint8_t font_cjk_bits[FONT_CJK_COUNT][24];
extern const uint8_t font_ascii_bits[FONT_ASCII_COUNT][12];

#endif /* FONT12_H */
