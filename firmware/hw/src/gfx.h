/**
  ******************************************************************************
  * @file    gfx.h
  * @brief   128x64 单色显存 + 5x7 ASCII 文字绘制
  ******************************************************************************
  */

#ifndef GFX_H
#define GFX_H

#include <stdint.h>

#define GFX_WIDTH  128
#define GFX_HEIGHT 64

void gfx_clear(void);
void gfx_pixel(int x, int y, int on);
void gfx_hline(int x0, int x1, int y);
void gfx_char(char ch, int x, int y);

/** @brief 一行文字占多少像素（每个字符 6 像素宽） */
int gfx_text_width(const char *s);

/** @brief 在 (x, y) 画一行文字，每个字符 6x8 像素 */
void gfx_text(int x, int y, const char *s);

/** @brief 水平居中画一行文字，y 是这一行最上面的像素坐标 */
void gfx_text_center(int y, const char *s);

/** @brief 返回 1024 字节显存（8 页 x 128 列） */
const uint8_t *gfx_buffer(void);

#endif /* GFX_H */
