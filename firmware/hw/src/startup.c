/**
  ******************************************************************************
  * @file    startup.c
  * @brief   最小启动文件：中断向量表 + 复位处理
  *
  * 芯片上电后，硬件从 Flash 起始地址 0x08000000 读两个值：
  *   [0] 初始栈顶指针（_estack）
  *   [1] Reset_Handler 地址
  * 然后跳到 Reset_Handler 执行。
  ******************************************************************************
  */

#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata;   /* .data 段在 Flash 里的初始值 */
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

int main(void);

void Reset_Handler(void);
void Default_Handler(void);
void SysTick_Handler(void);

void Default_Handler(void)
{
    while (1)
    {
    }
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    /* 1. 把 .data 段的初始值从 Flash 搬到 RAM */
    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    /* 2. .bss 段清零（全局变量默认是 0） */
    for (dst = &_sbss; dst < &_ebss; dst++)
    {
        *dst = 0;
    }

    /* 3. 进入 C 世界的 main */
    (void)main();

    while (1)
    {
    }
}

/* 中断向量表：放在 Flash 最前面（.isr_vector 段） */
__attribute__((section(".isr_vector"), used))
void (*const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),   /* 初始栈顶 */
    Reset_Handler,                /* 复位 */
    Default_Handler,              /* NMI */
    Default_Handler,              /* HardFault */
    Default_Handler,              /* MemManage */
    Default_Handler,              /* BusFault */
    Default_Handler,              /* UsageFault */
    0, 0, 0, 0,                   /* 保留 */
    Default_Handler,              /* SVCall */
    Default_Handler,              /* DebugMonitor */
    0,                            /* 保留 */
    Default_Handler,              /* PendSV */
    SysTick_Handler,              /* SysTick */

    /* 外设中断：F103C8T6 共 43 个（EXTI0_IRQn ... USBWakeUp_IRQn） */
    [16 ... 58] = Default_Handler
};
