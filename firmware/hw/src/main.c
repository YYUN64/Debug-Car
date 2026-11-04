#include "hw.h"
int main (void) {
	hw_init(); // 初始化
	for(;;){
		if(key1_pressed() == 1) {
			led_set(1);
		}
		else {
			led_set(0);
		}
	}
	return 0;
}
