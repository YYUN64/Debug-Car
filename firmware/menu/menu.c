#include <stdio.h>
// 先定义一些变量备用
#define MAX_SCREEN 7
char title[MAX_SCREEN][40] = { 
      // 第 0 屏    第 1 屏    第 2 屏     第 3 屏     第 4 屏
	"主菜单", "LED 控制", "信息显示", "巡线功能", "拓展功能",
      // 第 5 屏    第 6 屏
        " LED 1 ", " LED 2 ",
};
char name[MAX_SCREEN][4][40] = {
    { "LED 控制", "信息显示", "巡线功能", "拓展功能" }, // 第0屏 主菜单
    { "LED 1"   , "LED 2"   , "常亮"    , "交替"     }, // 第1屏 LED控制
    { "学号"    , "版本"    , "运行时间"             }, // 第2屏 信息显示
    { "启动巡线", "停止巡线", "巡线速度"             }, // 第3屏 巡线功能
    { "蜂鸣器"  , "舵机测试", "LED 流水灯"           }, // 第4屏 拓展功能
    { "开"      , "关"                               },	// 第5屏 灯一
    { "开"      , "关"                               },	// 第6屏 灯二
};
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
int child[MAX_SCREEN][4] = {
	{  1,  2,  3,  4}, // 主菜单
	{  5,  6, -1, -1}, // LED 控制
	{ -1, -1, -1, -1}, // 信息显示
	{ -1, -1, -1, -1}, // 巡线功能
	{ -1, -1, -1, -1}, // 拓展功能
	{ -1, -1, -1, -1}, // 灯一
	{ -1, -1, -1, -1}, // 灯二
};
int cursor[MAX_SCREEN] = { 0 };  //指针位置
int screen    = 0;               //屏数
				    
// 画出屏幕
void draw(void)
{
    int i;
    printf("\n=== %s ===\n", title[screen]);
    for (i = 0; i < count[screen]; i++) {
        if (i == cursor[screen]) printf(" > %s\n", name[screen][i]);
        else printf("   %s\n", name[screen][i]);
    }
}

// 主函数
int main(void)
{
    int next;
    int key; // 按键
    draw();  // 显示菜单
    while (scanf("%d", &key) == 1) { // input 
	// 向上走（循环）
        if (key == 1) {
		if(cursor[screen] == 0) cursor[screen] = count[screen] - 1;
		else cursor[screen]--;
	}
	// 向下走（循环）
        if (key == 2) {
		if(cursor[screen] == count[screen] - 1) cursor[screen] = 0;
        	else cursor[screen]++;
	}
	// 进入这一屏
        if (key == 3) {
		next = child[screen][cursor[screen]];
        	if (next >= 0) screen = next;
		else printf("  >> 执行：%s\n", name[screen][cursor[screen]]);
        }
	// 退出这一屏
        if (key == 4 && parent[screen] >= 0) {
		cursor[screen] = 0;
		screen = parent[screen];
	}
        draw(); // 重新画屏
    }
    return 0;
}
