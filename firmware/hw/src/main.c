#include "hw.h"

int main(void) {
	int k;
	int led1_on = 0;
	int led2_on = 0;
	uint32_t last = 0;

	hw_init();

	for (;;) {
		if (millis() - last >= 5) {
			last = millis();
			k = keys_scan();

			if (k == 1) {
				led1_on = 1 - led1_on;
				led1_set(led1_on);
			}
			if (k == 2) {
				led2_on = 1 - led2_on;
				led2_set(led2_on);
			}
		}
	}

	return 0;
}
