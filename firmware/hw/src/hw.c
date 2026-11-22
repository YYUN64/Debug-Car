#include "hw.h"
#include "regs.h"

static volatile uint32_t start_ms = 0;
void SysTick_Handler(void) { start_ms++ ; } // 负责计时从开机到现在的时间
// "& 与"  "| 或"  "~ 非"
void hw_init (void) {
	// 开 GPIOA、GPIOB 的时钟
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN;
	// RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	// RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	
	// GPIOA->CRL = GPIOA->CRL & ~(0xFU << 20); // PA5
	// GPIOA->CRL = GPIOA->CRL |  (0x2U << 20);
	GPIOA->CRL = (GPIOA->CRL & ~(0xFU << 20)) | (0x2U << 20); // PA5
	GPIOB->CRL = (GPIOB->CRL & ~(0xFU <<  4)) | (0x8U <<  4); // PB1
	GPIOB->ODR |= (1U << 1);     /* bit1 = 1 → PB1 高电平 */
	GPIOA->BRR  = (1U << 5);     /* bit5 = 0 → PA5 输出低 → 灯灭 */
	// GPIOA->BRR  = (1U << 5); 使用 = 相当于 &(~(1U << 5)) 是为了写0
	// 不支持直接写0，所以绕了一圈
	// BRR下会做ODR = ODR & (~写入值)这样的转换
	// ODR下不支持，要完整写出来
	// ------
	GPIOB->BRR  = (1U << 9);
	GPIOB->CRH  = (GPIOA->CRL & ~(0xFU << 4)) | (0x2U << 4);

	GPIOB->CRL  = (GPIOB->CRL & ~0xFU) | 0x8U;
	GPIOB->ODR |= (1U << 0);

	GPIOA->CRL  = (GPIOA->CRL & ~(0xFU << 24)) | (0x8U << 24);
	GPIOA->CRL  = (GPIOA->CRL & ~(0xFU << 28)) | (0x8U << 28);
	GPIOA->ODR |= (1U << 6) | (1U << 7);
	// ------
	SYSTICK->LOAD = 8000U - 1U;    // 8MHz ÷ 8000 = 1ms
	SYSTICK->VAL  = 0;             // 计数器清零
	// 使能 | 中断 | 内核时钟(8MHz)
	// bit0 | bit1 | bit2
	//  1   +  2   +  4  = 7U
	SYSTICK->CTRL = 7U;
	// SYSTICK->CTRL |= SYSTICK_CTRL_CLKSOURCE; // bit2：用 8MHz 内核时钟
	// SYSTICK->CTRL |= SYSTICK_CTRL_TICKINT;   // bit1：数到 0 进中断 
	// SYSTICK->CTRL |= SYSTICK_CTRL_ENABLE;    // bit0：开始计数,放最后
	// SYSTICK->CTRL = ENABLE | TICKINT | CLKSOURCE; // bit0 bit1 bit2
	// 用 |= 而不是 =：每一行只负责一个位，防止把上一行刚设的位冲掉
	// regs.h 定义了上面三个宏的值分别为4 2 1
}
// 输出模式下由BRR、BSRR决定电平
// 输入模式下由ODR决定电平

void led1_set(int on) {
	if (on == 1) { // PA5
		GPIOA->BSRR = (1U << 5); // 高电平、on
	}
	else {
		GPIOA->BRR  = (1U << 5); // 低电平、off
		// GPIOA->BSRR = (1U << 5 + 16);
	}
}

void led2_set(int on) {
	if (on == 1) { // PB9
		GPIOB->BSRR = (1U << 9);
	}
	else {
		GPIOB->BRR  = (1U << 9);
	}
}

int key1_pressed(void) {
	if ((GPIOB->IDR & (1U << 1)) == 0) return 1;
	else return 0;
}

uint32_t millis(void) { return start_ms; } // 负责读取开机了多久

void delay_ms(uint32_t ms) { // 负责等待差值
    uint32_t start = start_ms;
    while ((start_ms - start) < ms) { }
}

// 'SysTick_Handler'  'millis'  'delay_ms' 三个函数组成一个完整的时钟

// 消抖
static int key_stable[4];   /* 每个键确认过的状态：1=按着，0=松着 */
static int key_count[4];    /* 连续几次读到跟确认状态不一样 */

/* 按下返回 1：引脚被 GND 拉低，读到 0 才算按下 */
static int key_raw(int i) {
	if (i == 0) return ((GPIOB->IDR & (1U << 1)) == 0U); // PB1 = 键 1 
	if (i == 1) return ((GPIOB->IDR & (1U << 0)) == 0U); // PB0 = 键 2 
	if (i == 2) return ((GPIOA->IDR & (1U << 7)) == 0U); // PA7 = 键 3 
	if (i == 3) return ((GPIOA->IDR & (1U << 6)) == 0U); // PA6 = 键 4 
}

int keys_scan(void) {
	int i;
	int raw;
	for (i = 0; i < 4; i++) {
		raw = key_raw(i);
		if (raw != key_stable[i]) {
			key_count[i] = key_count[i] + 1;
			if (key_count[i] >= 3) {
				key_stable[i] = raw;
				key_count[i]  = 0;
				if (raw == 1) {
					return i + 1;
				}
			}
		}
		else {
			key_count[i] = 0;
		}
	}

	return 0;
}
