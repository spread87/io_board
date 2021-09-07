#include "sci.h"
#include "stm32f1xx.h"

__attribute__((weak)) void udelay(uint32_t xus)
{
	volatile int i;
	while(xus--) {
		for (i = 0; i < 10; i++) {
			__NOP();
		}
	}
}

__attribute__((weak)) void mdelay(uint32_t xms)
{
	while(xms --)
		udelay(1000);
}
