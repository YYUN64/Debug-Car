#include <stdio.h>

// 先定义一些变量备用
char title[5][20] = { 
      // 第 0 屏    第 1 屏    第 2 屏     第 3 屏     第 4 屏
	"主菜单", "LED 控制", "信息显示", "巡线功能", "拓展功能" 
};
char name[5][4][20] = {
    { "LED 控制", "信息显示", "巡线功能", "拓展功能" }, // 第0屏 
    { "LED1 开",  "LED1 关",  "LED2 开",  "LED2 关"  }, // 第1屏
    { "学号",     "版本",     "运行时间"             }, // 第2屏
    { "启动巡线", "停止巡线", "巡线速度"             }, // 第3屏
    { "蜂鸣器",   "舵机测试", "LED 流水灯"           }, // 第4屏
};
int count[5]  = { 4, 4, 3, 3, 3 };  //判断每一屏有几行
int parent[5] = { -1, 0, 0, 0, 0 }; //返回第几屏
int cursor[5] = { 0, 0, 0, 0, 0 };  //指针位置
int screen    = 0;                  //屏数
				    
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
        	if (screen == 0) screen = cursor[0] + 1;
		else printf("  >> 执行：%s\n", name[screen][cursor[screen]]);
        }
        if (key == 4 && parent[screen] >= 0) {
		cursor[screen] = 0;
		screen = parent[screen];
	}
        draw(); // 重新画屏
    }
    return 0;
}
