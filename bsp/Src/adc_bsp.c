#include "module.h"
#include "sci.h"
#include "adc_bsp.h"
#include "adc.h"
#include <string.h>

#define RAW2VOL(x)	((x) / 4096 * 1650)

static module_t adc_module = {
	.name = "adc",
	.private = (void *)(& (adc_data_t) {
		.adc_accuracy = 12,
	}),
};

static uint16_t local_adc_buffer[ADC_CHANNEL_NUM * CHANNEL_DEEP];

static void local_adc_init(void *thiz)
{
	static int init_done = 0;
	(void)thiz;

	if (init_done)
		return;

	adc_init(local_adc_buffer, ARRAY_SIZE(local_adc_buffer));
	phy_adc_start();
	init_done++;
}

static void adc_start(void *thiz)
{
	(void)thiz;
	phy_adc_start();
}

static void adc_stop(void *thiz)
{
	(void)thiz;
	phy_adc_stop();
}


static uint16_t get_adc_accuracy(void *thiz)
{

	module_t * mod;
	adc_data_t * data;
	if (!thiz) {
		sys_log_e("input a invalid parameter\r\n");
		while(1);
	}

	mod = (module_t *)thiz;

	data = mod->private;

	/* return percision value */
	return data->adc_accuracy;
}

static uint16_t get_adc_raw_val(void *thiz, int chn)
{
	int chn_index;
	uint32_t sum = 0;
	int i;

	(void)thiz;
	if (chn >= ADC_CHANNEL_NUM)
		return 0;

	chn_index = chn;

	for (i = 0; i < CHANNEL_DEEP; i++) {
		sum += local_adc_buffer[chn_index + i * ADC_CHANNEL_NUM];
	}

	return sum / CHANNEL_DEEP;
}

static uint16_t get_adc_vol_val(void *thiz, int chn)
{
	uint16_t raw_data;
	uint16_t temp;
	uint16_t real_data;
	
	real_data = get_adc_raw_val(thiz, 0);

	if (!real_data)
		return 0;

	raw_data = get_adc_raw_val(thiz, chn);

	temp = 1650.0 / real_data * raw_data;

	return temp;
}

static adc_ops ops = {
	.init = local_adc_init,
	.start = adc_start,
	.stop = adc_stop,
	.adc_raw_val = get_adc_raw_val,
	.adc_percision = get_adc_accuracy,
	.adc_vol_val = get_adc_vol_val,
};

int adc_module_register(void)
{
	adc_module.ops = (void *)&ops;

	module_register(ADC_MODULE, &adc_module);
}

device_initcall(adc_module_register);

