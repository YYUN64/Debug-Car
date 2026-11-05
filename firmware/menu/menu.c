#include "action.h"
#include "menu.h"
// 更新提示：原本在这的三个define移动到了menu.h
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
    { "开"      , "关"                               },	// 第5屏 LED 1
    { "开"      , "关"                               },	// 第6屏 LED 2
};
char lines[LINE][LINE_LEN]; // 给OLED屏幕输出文字使用，储存字符
int count[MAX_SCREEN]  = { 4, 4, 3, 3, 3, 2, 2 };  //判断每一屏有几行
int parent[MAX_SCREEN] = { // 回到哪
	-1, // 主菜单
	 0, // LED 控制
	 0, // 信息显示
	 0, // 巡线功能
	 0, // 拓展功能
	 1, // LED 1
	 1, // LED 2
};
int child[MAX_SCREEN][LINE] = {
	{  1,  2,  3,  4 }, // 主菜单
	{  5,  6, -1, -1 }, // LED 控制
	{ -1, -1, -1, -1 }, // 信息显示
	{ -1, -1, -1, -1 }, // 巡线功能
	{ -1, -1, -1, -1 }, // 拓展功能
	{ -1, -1, -1, -1 }, // LED 1
	{ -1, -1, -1, -1 }, // LED 2
};
int cursor[MAX_SCREEN] = { 0 };  // 指针位置
int screen    = 0;               // 屏数

static int need_draw = 1;

// 初始化
void menu_init (void) {
	screen = 0;
	for (int i = 0; i < MAX_SCREEN; i++) {
		cursor[i] = 0;
	}
	action_init(); // 在action.c中，用于初始化时间
	need_draw = 1;
}

// 处理按键
void menu_key (int key) {
	int next = -1; // 初始化
	switch (key) {

		// 向上走（循环）
		case 1:
		if(cursor[screen] == 0) {
			cursor[screen] = count[screen] - 1;
		}
		else cursor[screen]--;
		break;

		// 向下走（循环）
		case 2:
		if(cursor[screen] == count[screen] - 1) {
			cursor[screen] = 0;
		}
		else cursor[screen]++;
		break;

		// 进入这一屏 || 执行命令
		case 3:
		next = child[screen][cursor[screen]];
		if (next >= 0) screen = next;
		// 叶子项：交给 action.c
		else do_action(screen, cursor[screen]);
		break;

		// 退出这一屏
		case 4:
		if (parent[screen] >= 0) {
			cursor[screen] = 0;
			screen = parent[screen];
		}
		break;

		// 除上面以外的情况
		default:
		break;
	}
	need_draw = 1;
}


// 构建字符内容
static void build(void) {
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

// 渲染
void menu_draw(void) {
	build();
	menu_render(); // 渲染 取决于用什么机器
}

// LED 交替翻转 action.c
void menu_tick(int ms) {
	tick(ms);
}

// 询问是否要重新画屏
int menu_dirty(void) {
	int ready = need_draw;
	need_draw = 0;
	return ready;
}
