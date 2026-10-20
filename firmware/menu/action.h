#ifndef ACTION_H
#define ACTION_H

/* ============================================================
 *  action.h  ——  只有"声明"，没有真正执行的代码
 *
 *  为什么要这个文件？
 *    C 编译器是从上往下一行行读的。menu.c 里要调用 do_action()，
 *    就必须在调用之前先"见过"这个名字，否则报
 *        implicit declaration of function 'do_action'
 *    所以在 .h 里提前写一句"这个函数存在、参数是什么"，
 *    menu.c 只要 #include "action.h" 就算见过了。
 *
 *  打个比方：
 *      action.h = 餐厅的菜单（只有菜名和价格）
 *      action.c = 后厨（真正做菜的地方）
 * ============================================================ */

/* 开机时调一次：记下开机时刻，用来算"运行时间" */
void action_init(void);

/* 菜单里按 3 执行命令时调：
 *     scr  = 第几屏（menu.c 里的 screen）
 *     item = 这一屏的第几项（menu.c 里的 cursor[screen]） */
void do_action(int scr, int item);

/* 每次画屏时调一次：打印一行"灯和电机现在什么状态"。
 * 这个纯粹是方便你自己验收，用不上可以整段删掉。 */
void show_status(void);

#endif /* ACTION_H */
