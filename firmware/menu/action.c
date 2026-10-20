/* ============================================================
 *  action.c  ——  菜单里"按了 3 之后，到底做什么"
 *
 *  menu.c 负责的事：挪光标、判断这一项有没有下一屏。
 *  它发现这一项没有下一屏（child == -1）时，就调用
 *
 *        do_action(screen, cursor[screen]);
 *
 *  把"第几屏的第几项"这两个数字递过来，由这个文件决定干什么。
 *
 *  看这个文件的顺序：
 *      1. 最上面几个变量：灯和电机现在的状态
 *      2. action_init()  ：开机时记时间
 *      3. do_action()    ：★★★ 核心，7 个 case 对应 7 屏 ★★★
 *      4. show_status()  ：打一行状态，方便验收
 * ============================================================ */

#include <stdio.h>
#include <time.h>
#include "action.h"

/* ---- 硬件当前的状态 ----
 * 为什么要存？比如第 5 屏有"开"和"关"两项，不记住灯现在是亮还是灭，
 * 就没法判断该开还是该关。
 *
 * static 的意思是"只有这个文件能看见这几个变量"，
 * menu.c 想改也改不到，状态只能通过下面的 do_action() 改。
 * （去掉 static 也能跑，只是没那么保险。）
 */
static int led1_on     = 0;   /* LED1 亮着吗：0 灭，1 亮 */
static int led2_on     = 0;   /* LED2 亮着吗 */
static int led_alt_on  = 0;   /* 交替闪烁开着吗 */
static int motor_on    = 0;   /* 电机转着吗 */
static int motor_speed = 60;  /* 电机速度，60 到 100 */

static time_t start_time = 0; /* 开机时刻，"运行时间"要用它 */


/* 开机调一次，记下现在的时刻 */
void action_init(void)
{
    start_time = time(NULL);
}


/* ============================================================
 *  ★ 核心：靠 (scr, item) 两个数判断"现在在哪"，然后执行
 *
 *      scr  = 第几屏（0~6）
 *      item = 这一屏的第几项（0 开始）
 *
 *  两个数一凑，屏幕上唯一的一个位置就确定了：
 *      比如 scr=3, item=0 → 第 3 屏「巡线功能」的第 1 项「启动巡线」
 *
 *  下面 7 个 case 对应 7 屏，case 后面的数字就是 scr。
 *  第 0 屏（主菜单）不用写，因为它的 4 项全是"进下一屏"，
 *  根本不会调用到这里，会直接掉到最后的 default。
 * ============================================================ */
void do_action(int scr, int item)
{
    switch (scr) {

    case 1:                         /* ===== 第 1 屏：LED 控制 ===== */
        if (item == 2) {            /* 第 3 项：常亮 */
            led_alt_on = 0;         /* 先关掉交替，不然两个效果抢同一个灯 */
            led1_on = 1;
            led2_on = 1;
            printf("  >> LED1、LED2 常亮\n");
            /* 上真机时把这行换成：LED1_Set(1); LED2_Set(1); */
        }
        else if (item == 3) {       /* 第 4 项：交替 */
            led1_on = 0;
            led2_on = 0;
            led_alt_on = 1;
            printf("  >> LED 交替闪烁\n");
        }
        /* item 是 0 或 1 时是"进 LED1 / LED2 子屏"，走不到这里 */
        break;

    case 2:                         /* ===== 第 2 屏：信息显示 ===== */
        if (item == 0) {            /* 学号 */
            printf("  >> 学号：2023000000\n");
        }
        else if (item == 1) {       /* 版本 */
            printf("  >> 版本：v1.0\n");
        }
        else {                      /* 运行时间 */
            /* 现在时刻 - 开机时刻 = 跑了多少秒 */
            /* 上真机时把 time(NULL) 换成 HAL_GetTick() / 1000 */
            printf("  >> 已运行 %lu 秒\n",
                   (unsigned long)(time(NULL) - start_time));
        }
        break;

    case 3:                         /* ===== 第 3 屏：巡线功能 ===== */
        if (item == 0) {            /* 启动巡线 */
            motor_on = 1;
            printf("  >> 电机启动，速度 %d\n", motor_speed);
            /* 上真机时换成：Motor_Start(motor_speed); */
        }
        else if (item == 1) {       /* 停止巡线 */
            motor_on = 0;
            printf("  >> 电机停止\n");
        }
        else {                      /* 巡线速度：60 → 70 → … → 100 → 60 */
            motor_speed += 10;
            if (motor_speed > 100) motor_speed = 60;
            printf("  >> 速度 = %d\n", motor_speed);
        }
        break;

    case 4:                         /* ===== 第 4 屏：拓展功能 ===== */
        if (item == 0) {            /* 蜂鸣器 */
            printf("  >> 蜂鸣器：滴——\n");
            /* 上真机时换成：Buzzer_Beep(); */
        }
        else if (item == 1) {       /* 舵机测试 */
            printf("  >> 舵机：0 -> 90 -> 180 -> 0\n");
        }
        else {                      /* LED 流水灯 */
            printf("  >> LED 流水灯\n");
        }
        break;

    case 5:                         /* ===== 第 5 屏：LED1 开 / 关 ===== */
        led_alt_on = 0;             /* 单独点灯前先把交替关掉 */
        led1_on = (item == 0);      /* 第 1 项(item 0)是"开"，第 2 项是"关" */
        printf("  >> LED1 %s\n", led1_on ? "亮" : "灭");
        /* 上真机时换成：LED1_Set(led1_on); */
        break;

    case 6:                         /* ===== 第 6 屏：LED2 开 / 关 ===== */
        led_alt_on = 0;
        led2_on = (item == 0);
        printf("  >> LED2 %s\n", led2_on ? "亮" : "灭");
        break;

    default:                        /* 第 0 屏主菜单，或者屏号写错 */
        break;
    }
}


/* 画屏时调用，打印当前状态，方便自己验收动作有没有生效 */
void show_status(void)
{
    printf("  [状态] LED1=%s LED2=%s 交替=%s 电机=%s 速度=%d\n",
           led1_on    ? "亮" : "灭",
           led2_on    ? "亮" : "灭",
           led_alt_on ? "开" : "关",
           motor_on   ? "转" : "停",
           motor_speed);
}
