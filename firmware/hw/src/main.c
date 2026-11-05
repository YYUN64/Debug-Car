#include "hw.h"
int main (void) {
	hw_init(); // 初始化
	for(;;){
		if(key1_pressed() == 1) {
			led1_set(1);
		}
		else {
			led1_set(0);
		}
	}
	return 0;
}
