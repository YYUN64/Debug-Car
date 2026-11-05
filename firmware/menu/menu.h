#ifndef MENU_H
#define MENU_H

#define MAX_SCREEN 7
#define LINE       4
#define LINE_LEN   40

void menu_init(void);
void menu_key(int key);
void menu_draw(void);
int  menu_dirty(void);     /* 1 = 需要重画；读完自动清 0 */
void menu_tick(int ms);

/* 各平台自己实现：电脑用 printf，板子用 OLED */
void menu_render(void);

extern char title[MAX_SCREEN][LINE_LEN];
extern char lines[LINE][LINE_LEN];
extern int screen;

#endif
