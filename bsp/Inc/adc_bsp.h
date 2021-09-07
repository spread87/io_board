#ifndef __ADC_BSP_H__
#define __ADC_BSP_H__

#include "sci.h"
#include "stm32f1xx.h"


#define ADC_CHANNEL_NUM		8
#define CHANNEL_DEEP		6

typedef struct {
	void (* init) (void *thiz);
	void (* start) (void *thiz);
	void (* stop) (void *thiz);
	uint16_t (* adc_raw_val) (void *thiz, int chn);
	uint16_t (* adc_percision) (void *thiz);
	uint16_t (* adc_vol_val) (void *thiz, int chn);
}adc_ops;

typedef struct {
	uint16_t adc_accuracy;
}adc_data_t;

#endif
