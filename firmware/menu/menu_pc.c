#include <stdio.h>
#include "menu.h"
#include "action.h"

void menu_render(void)                 /* 原来 draw() 里那三行，原样搬过来 */
{
    int i;
    printf("\n=== %s ===\n", title[screen]);
    for (i = 0; i < LINE; i++) {
        if (lines[i][0] != '\0') printf("%s\n", lines[i]);
    }
    show_status();
}

int main(void)
{
    int key;
    menu_init();
    menu_draw();
    while (scanf("%d", &key) == 1) {
        menu_key(key);
        if (menu_dirty()) menu_draw();  /* 和板子主循环同一个形态 */
    }
    return 0;
}
