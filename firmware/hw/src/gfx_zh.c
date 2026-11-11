/**
  ******************************************************************************
  * @file    gfx_zh.c
  * @brief   中文版显存与文字绘制（UTF-8 + 12px 点阵字库）
  *
  * 和 gfx.c 的接口完全一样，编译时二选一：
  *   make            -> gfx.c    (5x7 ASCII 英文版)
  *   make LANG=zh    -> gfx_zh.c (12px 中文版)
  ******************************************************************************
  */

#include "gfx.h"
#include "font12.h"

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

/* ---------------- UTF-8 与字体查找 ---------------- */

static uint32_t utf8_next(const char **ps)
{
    const uint8_t *s = (const uint8_t *)*ps;
    uint32_t cp;

    if (s[0] < 0x80U)
    {
        cp = s[0];
        *ps += 1;
    }
    else if ((s[0] & 0xE0U) == 0xC0U)
    {
        cp = ((uint32_t)(s[0] & 0x1FU) << 6) | (uint32_t)(s[1] & 0x3FU);
        *ps += 2;
    }
    else if ((s[0] & 0xF0U) == 0xE0U)
    {
        cp = ((uint32_t)(s[0] & 0x0FU) << 12)
           | ((uint32_t)(s[1] & 0x3FU) << 6)
           | (uint32_t)(s[2] & 0x3FU);
        *ps += 3;
    }
    else
    {
        cp = '?';
        *ps += 1;
    }

    return cp;
}

static int cjk_index(uint32_t cp)
{
    int lo = 0;
    int hi = FONT_CJK_COUNT - 1;

    while (lo <= hi)
    {
        int mid = (lo + hi) / 2;

        if (font_cjk_cp[mid] == cp)
        {
            return mid;
        }
        if (font_cjk_cp[mid] < cp)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid - 1;
        }
    }

    return -1;
}

static void draw_ascii(uint8_t ch, int x, int y)
{
    if (ch < 32U || ch > 126U)
    {
        ch = '?';
    }

    const uint8_t *bits = font_ascii_bits[ch - 32U];

    for (int row = 0; row < 12; row++)
    {
        for (int col = 0; col < 8; col++)
        {
            if ((bits[row] & (uint8_t)(0x80U >> col)) != 0U)
            {
                gfx_pixel(x + col, y + row, 1);
            }
        }
    }
}

static void draw_cjk(int idx, int x, int y)
{
    const uint8_t *bits = font_cjk_bits[idx];

    for (int row = 0; row < 12; row++)
    {
        uint16_t v = (uint16_t)(((uint16_t)bits[row * 2] << 8) | bits[row * 2 + 1]);

        for (int col = 0; col < 12; col++)
        {
            if ((v & (uint16_t)(0x8000U >> col)) != 0U)
            {
                gfx_pixel(x + col, y + row, 1);
            }
        }
    }
}

void gfx_char(char ch, int x, int y)
{
    draw_ascii((uint8_t)ch, x, y);
}

int gfx_text_width(const char *s)
{
    int w = 0;

    while (*s != '\0')
    {
        uint32_t cp = utf8_next(&s);
        w += (cp < 0x80U) ? 8 : 12;
    }

    return w;
}

void gfx_text(int x, int y, const char *s)
{
    int cx = x;

    while (*s != '\0')
    {
        uint32_t cp = utf8_next(&s);

        if (cp < 0x80U)
        {
            draw_ascii((uint8_t)cp, cx, y);
            cx += 8;
        }
        else
        {
            int idx = cjk_index(cp);

            if (idx >= 0)
            {
                draw_cjk(idx, cx, y);
            }
            else
            {
                draw_ascii('?', cx, y);
            }
            cx += 12;
        }
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
