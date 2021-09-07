#ifndef __MODBUS_HAL_H__
#define __MODBUS_HAL_H__

#include "stm32f1xx.h"

/* modbus mode define */
#define DRIVING		1
#define SLAVE		0

void modbus_low_handler_init(void);

void * modbus_device_id_binding(int if_num, uint16_t id, 
		int (*id_func)(void *, uint8_t *, uint16_t),
	void *parm);
int modbus_write(const void *thiz, uint8_t * data, uint16_t len);
#endif

