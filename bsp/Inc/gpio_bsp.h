#ifndef __GPIO_BSP_H__
#define __GPIO_BSP_H__

#include "stm32f103xb.h"

enum GPIO_FUNC_KEY {
	P_RS485_RW_CTRL = 0,
	P_RS485_2_RW_CTRL,
	P_RS485_3_RW_CTRL,
	P_LED_1_CTRL,
	P_LED_2_CTRL,
	P_OV,
	P_SW,
	P_DO0,
	P_DO1,
	P_DO2,
	P_DO3,
	P_DI0,
	P_DI1,
	P_DI2,
	P_DI3,
	P_EEPROM_SCL,
	P_EEPROM_SDA,
	P_DGB,
	P_DrvCH,
	P_DISCH,
	P_VTEMP1,
	P_VTEMP2,
	P_VTEMP3,
	P_VTEMP4,
	P_VBUS,
	P_Isensor1,
	P_Isensor2,
	P_VREF,
	P_SWSEL,
};

struct GPIO_ops {
	void (*init) (void *thiz);
	void (*write) (void *thiz, enum GPIO_FUNC_KEY key, void *value); 
	int (*read) (void *thiz, enum GPIO_FUNC_KEY key);
	int (*mode) (void *thiz, enum GPIO_FUNC_KEY, uint32_t mode);
};

#endif

