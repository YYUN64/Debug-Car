#include "menu.h"
#include "gfx.h"
#include "oled.h"

void menu_render(void)
{
	int i;

	gfx_clear();                          /* 清显存 */

	gfx_text_center(0, title[screen]);    /* 标题居中，y=0 贴顶 */
	gfx_hline(0, GFX_WIDTH - 1, 9);       /* 9 号行画一条分隔线 */

	for (i = 0; i < LINE; i++) {
		if (lines[i][0] != '\0') {    /* 空行不画 */
			gfx_text(0, 13 + i * 12, lines[i]);
		}
	}

	oled_flush();                         /* 整屏推给屏幕，约 150ms */
}
