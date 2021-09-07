#include "thermal_bsp.h"
#include "sci.h"
#include "module.h"
#include "adc_bsp.h"


static int const temp_lookup_table[][2] = {
	{0, 1650},
	{2000, 1590},
	{3000, 1559},
	{4000, 1516},
	{5000, 1459},
	{6000, 1386},
	{7000, 1297},
	{8000, 1193},
	{9000, 1079},
	{10000, 957},
	{11000, 836},
	{12000, 718},
	{12800, 630},
};

module_t thm_mod = {
	.name = "THERMAL",
};

typedef struct {
	int thm_id;
	void * mod_inst;
	struct {
		int const *t_to_v;
		int temp_table_num;
	}table;
	int adc_chn;
}thm_dev_t;

typedef struct {
	int dev_num;
	void *dev;
	void *adc_mod;
}thm_private_t;

#define THM_DRV(x) ((module_t *)(x))

thm_dev_t thm_dev[TEMPMAX] = {
	{
		.thm_id = 0,
		.adc_chn = 4,
	},
};

static thm_private_t thm_priv = {
	.dev_num = sizeof(thm_dev) / sizeof(thm_dev[0]),
};

static void init(void *thiz)
{
	thm_private_t *priv;
	thm_dev_t *dev;
	module_t * adc_mod;
	module_t * mod;
	adc_ops * adc_ops;
	int i, dev_num;
	
	if (!thiz) {
		sys_log_e("input parameter is NULL!!\r\n");
		while(1);
	}

	mod = thiz;
	priv = mod->private;
	dev_num = priv->dev_num;
	adc_mod = module_get(0, "adc");

	if (!adc_mod)
		while(1);

	adc_ops = adc_mod->ops;
	adc_ops->init(adc_mod);
	adc_ops->start(adc_mod);

	priv->adc_mod = adc_mod;
	priv->dev = thm_dev;

	for (i = 0; i < TEMPMAX; i++) {
		dev = (thm_dev_t *)((uint32_t)priv->dev + sizeof(thm_dev_t) * i);
		dev->table.t_to_v = &temp_lookup_table[0][0];
		dev->table.temp_table_num = sizeof(temp_lookup_table) / sizeof(temp_lookup_table[0]);
	}
}

static int adc_to_temp(const int *table, int table_num, int val)
{
	int temp, temp_hi, temp_lo, adc_hi, adc_lo;
	int i;
	int quot, rem;

	if (!table)
		return -1;

	for (i = 0; i < table_num; i++) {
		if (val >= table[2 * i + 1])
			break;
	}

	if (i == 0) {
		temp = table[0];
	} else if ( i >= table_num) {
		temp = table[2 * (table_num - 1)];
	} else {
		adc_hi = table[2 * i - 1];
		adc_lo = table[2 * i + 1];

		temp_hi = table[2 * i - 2];
		temp_lo = table[2 * i];

		temp = temp_hi + (	\
				{										\
				quot = (temp_lo - temp_hi) / (adc_lo - adc_hi);	\
				rem = (temp_lo - temp_hi) % (adc_lo - adc_hi);	\
				(quot * (val - adc_hi)) + ((rem * (val - adc_hi)) / (adc_lo - adc_hi)); \
				}										\
				);

	}
	return temp;
}

static int read_temp(void *thiz, int type)
{
	thm_private_t * priv;
	int raw_data;
	adc_ops *ops;
	thm_dev_t *dev;

	if (!thiz) {
		return -1;
	}
	priv = THM_DRV(thiz)->private;

	ops = THM_DRV(priv->adc_mod)->ops;

	if (type >= TEMPMAX)
		return -1;

	dev = &((thm_dev_t *)priv->dev)[type];

	raw_data = ops->adc_vol_val(priv->adc_mod, dev->adc_chn);

	return adc_to_temp(dev->table.t_to_v, dev->table.temp_table_num, raw_data);
}

static thm_ops_t ops = {
	.init = init,
	.read_temp = read_temp,

};

static int thm_module_register(void)
{
	thm_priv.dev_num = sizeof(thm_dev) / sizeof(thm_dev[0]);
	thm_mod.private = (void *)&thm_priv;
	thm_mod.ops = (void *)&ops;

	module_register(THERMAL_MODULE, &thm_mod);

	return 0;
}

device_initcall(thm_module_register);

