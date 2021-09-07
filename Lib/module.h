#ifndef __MODULE_H__
#define __MODULE_H__

#include "FreeRTOS.h"
#include "list.h"

#define module_list				List_t
#define module_list_item		ListItem_t

enum {
	EXAMPLER_MODULE = 0,
	GPIO_MODULE,
	RS485_IF_MODULE,
	WDG_MODULE,
	TIM_1_MODULE,
	TIM_2_MODULE,
	TIM_3_MODULE,
	UART_1_MODULE,
	UART_2_MODULE,
	ADC_MODULE,
	MAX_MODULE,
	EXTI_MODULE,
	COUNTER_MODULE,
	THERMAL_MODULE,
	ELEC_MODULE,
	I2C_MODULE,
	EEPROM_MODULE,
};

typedef struct {
	char name[10];
	
	void * private;
	void * ops;

	void (* probe) (void *thiz);
	module_list_item list_item;
}module_t;

int module_register(int flag, module_t *module);
module_t * module_get(int flag, const char *name);
void all_module_register(void);
int system_module_init(void);

#endif

