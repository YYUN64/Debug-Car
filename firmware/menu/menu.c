#include <stdio.h>
#include "action.h"   // ← 新加：动作都写在 action.c 里，这里只留声明
// 先定义一些变量备用
#define MAX_SCREEN 7
#define LINE_LEN 40
#define LINE 4
char title[MAX_SCREEN][LINE_LEN] = { 
      // 第 0 屏    第 1 屏    第 2 屏     第 3 屏     第 4 屏
	"主菜单", "LED 控制", "信息显示", "巡线功能", "拓展功能",
      // 第 5 屏    第 6 屏
        " LED 1 ", " LED 2 ",
};
char name[MAX_SCREEN][LINE][LINE_LEN] = {
    { "LED 控制", "信息显示", "巡线功能", "拓展功能" }, // 第0屏 主菜单
    { "LED 1"   , "LED 2"   , "常亮"    , "交替"     }, // 第1屏 LED控制
    { "学号"    , "版本"    , "运行时间"             }, // 第2屏 信息显示
    { "启动巡线", "停止巡线", "巡线速度"             }, // 第3屏 巡线功能
    { "蜂鸣器"  , "舵机测试", "LED 流水灯"           }, // 第4屏 拓展功能
    { "开"      , "关"                               },	// 第5屏 灯一
    { "开"      , "关"                               },	// 第6屏 灯二
};
char lines[LINE][LINE_LEN]; // 给OLED屏幕输出文字使用，储存字符
int count[MAX_SCREEN]  = { 4, 4, 3, 3, 3, 2, 2 };  //判断每一屏有几行
int parent[MAX_SCREEN] = { // 回到哪
	-1, // 主菜单
	 0, // LED 控制
	 0, // 信息显示
	 0, // 巡线功能
	 0, // 拓展功能
	 1, // 灯一
	 1, // 灯二
};
int child[MAX_SCREEN][LINE] = {
	{  1,  2,  3,  4}, // 主菜单
	{  5,  6, -1, -1}, // LED 控制
	{ -1, -1, -1, -1}, // 信息显示
	{ -1, -1, -1, -1}, // 巡线功能
	{ -1, -1, -1, -1}, // 拓展功能
	{ -1, -1, -1, -1}, // 灯一
	{ -1, -1, -1, -1}, // 灯二
};
int cursor[MAX_SCREEN] = { 0 };  // 指针位置
int screen    = 0;               // 屏数

// 构建字符内容
void build(void) 
{
    int i, j, n;
    for (i = 0; i < LINE; i++) { // 判断这个OLED屏幕有多少行数
    	if(i < count[screen]) { // 判断这一屏有多少有效文本输出
		lines[i][0] = ' '; // 第零个字符是' '
		                   // 决定第一字符填入什么
		if(i == cursor[screen]) lines[i][1] = '>';
		else                    lines[i][1] = ' ';
		lines[i][2] = ' '; // 第二个字符是' '
		                   // 效果' > ' || '   '
		n = 3;             // 前面三个空都有内容了，从第四个空开始填
		                   // 正式填入文字部分
		for (j = 0; name[screen][i][j] != '\0' ; j++) {
			lines[i][n] = name[screen][i][j];
			n++;
		}
		                   // 让末尾字符是'\0'
		lines[i][n] = '\0';
	}
	else { // 发现这一行没有任何有效内容，直接让第零个字符为'\0'
		lines[i][0] = '\0'; 
	} // 这么写好看一点，可以不使用{}
    }
}
// 画出屏幕
// 把这一屏送到屏幕：电脑上用 printf，真机上换成 OLED 绘制
void draw(void)
{
    int i;
    printf("\n=== %s ===\n", title[screen]);
    for (i = 0; i < LINE; i++) {
    	if(lines[i][0] != '\0') printf("%s\n", lines[i]);
    }
    show_status();   // ← 新加：把灯和电机的状态打一行出来，方便验收
}

// 主函数
int main(void)
{
	int next;
    int key; // 按键
    action_init();  // ← 新加：让 action.c 记下启动时刻
    build();
    draw();  // 显示菜单
    while (scanf("%d", &key) == 1) { // input 
		switch (key) {

			// 向上走（循环）
			case 1:
				if(cursor[screen] == 0) cursor[screen] = count[screen] - 1;
				else cursor[screen]--;
				break;

			// 向下走（循环）
			case 2:
				if(cursor[screen] == count[screen] - 1) cursor[screen] = 0;
       			else cursor[screen]++;
				break;
				
			// 进入这一屏 || 执行命令
			case 3:
				next = child[screen][cursor[screen]];
       			if (next >= 0) screen = next;
				else do_action(screen, cursor[screen]); // ← 叶子项：交给 action.c
				break;

			// 退出这一屏
			case 4:
				if (parent[screen] >= 0) {
					cursor[screen] = 0;
					screen = parent[screen];
				}
				break;

			default:
				break;
		
		}
		build();
        draw(); // 重新画屏
    }
    return 0;
}
