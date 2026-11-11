/**
  ******************************************************************************
  * @file    oled.c
  * @brief   软件 I2C（PB11=SCL, PB10=SDA, ~100kHz）+ SSD1306 初始化、按页刷新
  *
  * 直接用寄存器操作，不依赖 HAL。I2C 时序：HSI 8MHz 下 CCR=40 -> 100kHz。
  ******************************************************************************
  */

#include "oled.h"
#include "gfx.h"
#include "regs.h"
#include "hw.h"

#define SSD1306_ADDR_3C   0x3CU
#define SSD1306_ADDR_3D   0x3DU

static uint8_t s_addr = 0;
static uint8_t s_tx[1 + GFX_WIDTH];

/* SSD1306 初始化命令序列 */
static const uint8_t k_init_seq[] =
{
    0xAE,        /* 关显示 */
    0x20, 0x00,  /* 水平寻址模式 */
    0xB0,        /* 页地址 = 0 */
    0xC8,        /* COM 扫描方向：从下到上 */
    0x00, 0x10,  /* 列地址 = 0 */
    0x40,        /* 显示起始行 = 0 */
    0x81, 0xCF,  /* 对比度 */
    0xA1,        /* 段重映射：左右不镜像 */
    0xA6,        /* 正常显示 */
    0xA8, 0x3F,  /* 多路复用 = 64 */
    0xA4,        /* 输出跟随显存 */
    0xD3, 0x00,  /* 显示偏移 = 0 */
    0xD5, 0x80,  /* 时钟分频 */
    0xD9, 0xF1,  /* 预充电 */
    0xDA, 0x12,  /* COM 引脚配置 */
    0xDB, 0x40,  /* VCOMH */
    0x8D, 0x14,  /* 电荷泵开 */
    0xAF         /* 开显示 */
};

/* ---------------- 软件 I2C（PB11=SCL, PB10=SDA） ---------------- */

#define SCL_PIN    11U
#define SDA_PIN    10U
#define I2C_DELAY  5

/* 粗略延时：HSI 8MHz 下每圈约 5 个周期，5 圈 ≈ 3µs */
static void i2c_delay(void)
{
    for (volatile int i = 0; i < I2C_DELAY; i++)
    {
    }
}

static void scl_set(int high)
{
    if (high)
    {
        GPIOB->BSRR = (1UL << SCL_PIN);   /* 释放，由模块上的上拉拉高 */
    }
    else
    {
        GPIOB->BRR = (1UL << SCL_PIN);    /* 主动拉低 */
    }
}

static void sda_set(int high)
{
    if (high)
    {
        GPIOB->BSRR = (1UL << SDA_PIN);
    }
    else
    {
        GPIOB->BRR = (1UL << SDA_PIN);
    }
}

static void i2c_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
    (void)RCC->APB2ENR;

    /* PB10/PB11：开漏输出（0x6），空闲时两条线都由上拉拉高 */
    GPIOB->BSRR = (1UL << SCL_PIN) | (1UL << SDA_PIN);
    GPIOB->CRH  = (GPIOB->CRH & ~(0xFFUL << 8)) | (0x66UL << 8);
}

static void i2c_start(void)
{
    sda_set(1);
    scl_set(1);
    i2c_delay();
    sda_set(0);        /* SCL 高时 SDA 下降 = START */
    i2c_delay();
    scl_set(0);
    i2c_delay();
}

static void i2c_stop(void)
{
    sda_set(0);
    i2c_delay();
    scl_set(1);
    i2c_delay();
    sda_set(1);        /* SCL 高时 SDA 上升 = STOP */
    i2c_delay();
}

/* 返回 1 = 收到 ACK，0 = 没应答 */
static int i2c_read_ack(void)
{
    int ack;

    /* SDA 临时改成上拉输入，释放总线等从机拉低 */
    GPIOB->BSRR = (1UL << SDA_PIN);
    GPIOB->CRH  = (GPIOB->CRH & ~(0xFUL << 8)) | (0x8UL << 8);
    i2c_delay();

    scl_set(1);
    i2c_delay();
    ack = ((GPIOB->IDR & (1UL << SDA_PIN)) == 0U) ? 1 : 0;
    scl_set(0);
    i2c_delay();

    /* 改回开漏输出 */
    GPIOB->CRH  = (GPIOB->CRH & ~(0xFUL << 8)) | (0x6UL << 8);
    GPIOB->BSRR = (1UL << SDA_PIN);
    i2c_delay();

    return ack;
}

/* 返回 1 = ACK，0 = NACK */
static int i2c_write_byte(uint8_t b)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        if ((b & 0x80U) != 0U)
        {
            sda_set(1);
        }
        else
        {
            sda_set(0);
        }
        b = (uint8_t)(b << 1);

        i2c_delay();
        scl_set(1);
        i2c_delay();
        scl_set(0);
        i2c_delay();
    }

    return i2c_read_ack();
}

static int i2c_write(uint8_t addr7, const uint8_t *data, uint16_t n)
{
    uint16_t i;

    i2c_start();
    if (i2c_write_byte((uint8_t)(addr7 << 1)) == 0)
    {
        i2c_stop();
        return -1;
    }

    for (i = 0; i < n; i++)
    {
        if (i2c_write_byte(data[i]) == 0)
        {
            i2c_stop();
            return -2;
        }
    }

    i2c_stop();
    return 0;
}

static int send_cmds(const uint8_t *cmds, uint16_t n)
{
    uint8_t buf[32];
    uint16_t i;

    if (n > (uint16_t)(sizeof(buf) - 1U))
    {
        return -1;
    }

    buf[0] = 0x00;
    for (i = 0; i < n; i++)
    {
        buf[i + 1U] = cmds[i];
    }

    return i2c_write(s_addr, buf, (uint16_t)(n + 1U));
}

static int ssd1306_start(uint8_t addr7)
{
    s_addr = addr7;

    if (send_cmds(k_init_seq, (uint16_t)sizeof(k_init_seq)) != 0)
    {
        return -1;
    }

    /* 清屏 */
    gfx_clear();
    oled_flush();
    return 0;
}

/* ---------------- 对外接口 ---------------- */

uint8_t oled_init(void)
{
    i2c_init();
    delay_ms(100);

    if (ssd1306_start(SSD1306_ADDR_3C) == 0)
    {
        return SSD1306_ADDR_3C;
    }
    if (ssd1306_start(SSD1306_ADDR_3D) == 0)
    {
        return SSD1306_ADDR_3D;
    }
    return 0;
}

void oled_flush(void)
{
    const uint8_t *fb = gfx_buffer();

    s_tx[0] = 0x40;

    for (uint8_t page = 0; page < 8U; page++)
    {
        uint8_t cmds[3];
        cmds[0] = (uint8_t)(0xB0U | page);
        cmds[1] = 0x00;
        cmds[2] = 0x10;

        if (send_cmds(cmds, 3) != 0)
        {
            return;
        }

        for (uint16_t i = 0; i < GFX_WIDTH; i++)
        {
            s_tx[i + 1U] = fb[(uint16_t)page * GFX_WIDTH + i];
        }

        if (i2c_write(s_addr, s_tx, GFX_WIDTH + 1U) != 0)
        {
            return;
        }
    }
}
