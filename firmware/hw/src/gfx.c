/**
  ******************************************************************************
  * @file    gfx.c
  * @brief   显存与 5x7 ASCII 文字绘制
  *
  * 显存格式和 SSD1306 一致：8 页，每页 128 字节，一个字节管竖着的 8 个像素。
  ******************************************************************************
  */

#include "gfx.h"
#include "font5x7.h"

static uint8_t s_fb[GFX_WIDTH * GFX_HEIGHT / 8];

void gfx_clear(void)
{
    for (uint32_t i = 0; i < sizeof(s_fb); i++)
    {
        s_fb[i] = 0;
    }
}

void gfx_pixel(int x, int y, int on)
{
    if (x < 0 || x >= GFX_WIDTH || y < 0 || y >= GFX_HEIGHT)
    {
        return;
    }

    uint16_t index = (uint16_t)x + (uint16_t)(y / 8) * GFX_WIDTH;
    uint8_t  mask  = (uint8_t)(1U << (y % 8));

    if (on)
    {
        s_fb[index] |= mask;
    }
    else
    {
        s_fb[index] &= (uint8_t)~mask;
    }
}

void gfx_hline(int x0, int x1, int y)
{
    for (int x = x0; x <= x1; x++)
    {
        gfx_pixel(x, y, 1);
    }
}

void gfx_char(char ch, int x, int y)
{
    if ((uint8_t)ch < 0x20U || (uint8_t)ch > 0x7FU)
    {
        ch = '?';
    }

    const uint8_t *glyph = font5x7[(uint8_t)ch - 0x20U];

    for (int col = 0; col < 5; col++)
    {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 7; row++)
        {
            if ((bits & (uint8_t)(1U << row)) != 0U)
            {
                gfx_pixel(x + col, y + row, 1);
            }
        }
    }
}

int gfx_text_width(const char *s)
{
    int n = 0;

    while (s[n] != '\0')
    {
        n++;
    }

    return n * 6;
}

void gfx_text(int x, int y, const char *s)
{
    int cx = x;

    while (*s != '\0')
    {
        gfx_char(*s, cx, y);
        cx += 6;
        s++;
    }
}

void gfx_text_center(int y, const char *s)
{
    int x = (GFX_WIDTH - gfx_text_width(s)) / 2;

    if (x < 0)
    {
        x = 0;
    }

    gfx_text(x, y, s);
}

const uint8_t *gfx_buffer(void)
{
    return s_fb;
}
