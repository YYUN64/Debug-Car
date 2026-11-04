/**
  ******************************************************************************
  * @file    regs.h
  * @brief   只用到的几个外设寄存器定义（不依赖 CMSIS / HAL）
  *
  * STM32F103 的外设寄存器地址是固定的，这里用结构体"贴"到对应地址上，
  * 就能像 RCC->APB2ENR 这样访问。地址来自 RM0008 存储映射表。
  ******************************************************************************
  */

#ifndef REGS_H
#define REGS_H

#include <stdint.h>

/* ---------------- 外设结构体 ---------------- */

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

typedef struct
{
    volatile uint32_t CRL;    /* 引脚 0~7 的配置 */
    volatile uint32_t CRH;    /* 引脚 8~15 的配置 */
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_TypeDef;

typedef struct
{
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
} SysTick_TypeDef;

/* ---------------- 外设基地址 ---------------- */

#define RCC      ((RCC_TypeDef *)      0x40021000UL)
#define GPIOA    ((GPIO_TypeDef *)     0x40010800UL)
#define GPIOB    ((GPIO_TypeDef *)     0x40010C00UL)
#define GPIOC    ((GPIO_TypeDef *)     0x40011000UL)
#define I2C1     ((I2C_TypeDef *)      0x40005400UL)   /* 旧接线（PB6/PB7），现在不用 */
#define I2C2     ((I2C_TypeDef *)      0x40005800UL)   /* 备用：现在用软件 I2C（PB11=SCL / PB10=SDA） */
#define SYSTICK  ((SysTick_TypeDef *)  0xE000E010UL)

/* ---------------- 寄存器位 ---------------- */

#define RCC_APB2ENR_AFIOEN      (1UL << 0)
#define RCC_APB2ENR_IOPAEN      (1UL << 2)
#define RCC_APB2ENR_IOPBEN      (1UL << 3)
#define RCC_APB2ENR_IOPCEN      (1UL << 4)
#define RCC_APB1ENR_I2C1EN      (1UL << 21)
#define RCC_APB1ENR_I2C2EN      (1UL << 22)

#define I2C_CR1_PE              (1UL << 0)
#define I2C_CR1_START           (1UL << 8)
#define I2C_CR1_STOP            (1UL << 9)

#define I2C_SR1_SB              (1UL << 0)
#define I2C_SR1_ADDR            (1UL << 1)
#define I2C_SR1_BTF             (1UL << 2)
#define I2C_SR1_TXE             (1UL << 7)
#define I2C_SR1_AF              (1UL << 10)

#define SYSTICK_CTRL_ENABLE     (1UL << 0)
#define SYSTICK_CTRL_TICKINT    (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1UL << 2)
#define SYSTICK_CTRL_COUNTFLAG  (1UL << 16)

/* 板载 LED：PC13，低电平点亮 */
#define LED_PIN                 13U

#endif /* REGS_H */
