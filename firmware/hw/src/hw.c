#include "hw.h"
#include "regs.h"
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

int key1_pressed(void) {
	if ((GPIOB->IDR & (1U << 1)) == 0) return 1;
	else return 0;
}
