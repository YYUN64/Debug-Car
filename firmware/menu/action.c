#include <stdio.h>
#include <time.h>
#include "action.h"

static int led_pwr[2]  = {0}; // LED 电源 0 灭 1 亮
static int led_shift   = 0;   // 交替闪烁
static int led_ms      = 0;   // 距离上次切换等待了多久
static int led_on      = 0;   // LED
static int motor_pwr   = 0;   // 电机旋转
static int motor_speed = 60;  // 电机速度，60 到 100

// Time Record
static time_t start_time = 0; // 开机
void action_init(void) {      // 初始化
    start_time = time(NULL);
}

// Action
void do_action(int oled, int arrow) {
	switch(oled) {

		// LED控制
		case 1:
		if(arrow == 2) { // 屏1 常亮选项
			led_shift = 0;
			led_pwr[0] = 1;
			led_pwr[1] = 1;
			printf("  >> LED 1、LED 2 常亮\n");
		}
		else if(arrow == 3) { // 屏2 交替选项
			led_ms = 0; // 重置一下
			led_on = 0; //
			led_pwr[0] = 1;
			led_pwr[1] = 0;
			led_shift = 1;
			printf("  >> LED 交替闪烁\n");
		}
		break;

		// 信息显示
		case 2:
		if(arrow == 0) {
			printf("  >> 学号：2023000000\n");
		}
		else if(arrow == 1) {
			printf("  >> 版本：v1.0\n");
		}
		else if(arrow == 2) {
			printf("  >> 已运行 %lu 秒\n",
				(unsigned long)(time(NULL) - start_time));
		}
		break;

		// 巡线功能
		case 3:
		if(arrow == 0) {
			motor_pwr = 1;
			printf("  >> 电机启动，速度 %d\n", motor_speed);
		}
		else if(arrow == 1) {
			motor_pwr = 0;
			printf("  >> 电机停止\n");
		}
		else if(arrow == 2) {
			if(motor_speed == 100) motor_speed = 60;
			else motor_speed += 10;
			printf("  >> 速度 = %d\n", motor_speed);
		}
		break;

		// 拓展功能
		case 4:
		if(arrow == 0) {
			printf("  >> 蜂鸣器：滴——\n");
		}
		else if(arrow == 1) {
			printf("  >> 舵机：0 -> 90 -> 180 -> 0\n");
		}
		else if(arrow == 2) {
			printf("  >> LED 流水灯\n");
		}
		break;

		// LED 1
		case 5:
		{
			led_shift = 0;               // 交替关闭
			led_pwr[0] = (arrow == 0);   // 开(True) 关(False)
			printf("  >> LED 1 %s\n", led_pwr[0] ? "亮" : "灭");
			/* 上真机时换成：LED1_Set(led_pwr[0]); */
			/* 条件表达式 ? 表达式1 : 表达式2 */
			break;
		}

		// LED 2
		case 6:
		{
			led_shift = 0;               // 交替关闭
			led_pwr[1] = (arrow == 0);   // 开(True) 关(False)
			printf("  >> LED 2 %s\n", led_pwr[1] ? "亮" : "灭");
			/* 上真机时换成：LED1_Set(led_pwr[1]); */
			break;
		}

		// Default ( 第 0 屏主菜单，或者屏号写错 )
		default:
		{
			break;
		}
	}
}

void tick(int ms) {                   // 输入ms
	if(led_shift == 0) return;   // 判断交替是否
	led_ms += ms;                 // 计时
	if(led_ms < 500) return;      // 判断是否到了该切换的时候
	led_ms -= 500;                //
	led_on = 1 - led_on;          // 切换 (static int led_on = 0;)
	for(int i = 0; i < 2; i++) {
		led_pwr[i] = (led_on == i);
	}
	printf("  >> 交替：LED 1 %s LED 2 %s\n",
			led_pwr[0] ? "亮" : "灭", led_pwr[1] ? "亮" : "灭");
}
/*
使用 if + else 便于理解，3个版本后删除
void tick(int ms)
{
	if (led_alt_on == 0) return;
	else {
		led_alt_ms += ms;
		if (led_alt_ms < 500) return;
		else {
			led_alt_ms -= 500;
			led_alt_phase = 1 - led_alt_phase;

			for(int i = 0; i < 2; i++) {
				led_pwr[i] = (led_alt_phase == i);
			}
			printf("  >> 翻转：LED 1=%s，LED 2=%s\n",
				led1_on ? "亮" : "灭", led2_on ? "亮" : "灭");
		}
	}
}
*/

/* 画屏时调用，打印当前状态，方便自己验收动作有没有生效 */
void show_status(void)
{
    printf("  [状态] LED 1=%s LED 2=%s 交替=%s 电机=%s 速度=%d\n",
           led_pwr[0] ? "亮" : "灭",
           led_pwr[1] ? "亮" : "灭",
           led_shift  ? "开" : "关",
           motor_pwr  ? "转" : "停",
           motor_speed
        );
}
