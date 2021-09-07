#include "cur_vol_bsp.h"
#include "sci.h"
#include "module.h"
#include "adc_bsp.h"

module_t elec_mod = {
	.name = "ELECTRON",
};

typedef struct {
	void * mod_inst;
	int idel_k;
	int idel_b;
	int adc_chn;
	int zero_cali_para;
	int cali_para;
}elec_dev_t;

typedef struct {
	int dev_num;
	void *dev;
	void *adc_mod;
}elec_private_t;

#define ELEC_DRV(x) ((module_t *)(x))

elec_dev_t elec_dev[ELECMAX] = {
	{
		.idel_k = 167,
		.idel_b = 0,
		.adc_chn = 1,
		.cali_para = 1000,
		.zero_cali_para = 1650,
	},
	{
		.idel_k = 367,
		.idel_b = 0,
		.adc_chn = 2,
		.cali_para = 1000,
		.zero_cali_para = 1650,
	},
	{
		.idel_k = 2200,
		.idel_b = 0,
		.adc_chn = 3,
		.cali_para = 1000,
		.zero_cali_para = 0,
	},
};

static elec_private_t elec_priv = {
	.dev_num = sizeof(elec_dev) / sizeof(elec_dev[0]),
};

static void init(void *thiz)
{
	elec_private_t *priv;
	elec_dev_t *dev;
	module_t * adc_mod;
	module_t * mod;
	int i, dev_num;
	adc_ops * adc_ops;
	
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
	priv->dev = elec_dev;
	priv->adc_mod = adc_mod;

	for (i = 0; i < dev_num; i++) {
		dev = (elec_dev_t *)((uint32_t)priv->dev + sizeof(elec_dev_t) * i);
	}
}

static int read_elec(void *thiz, int type)
{
	elec_private_t * priv;
	int raw_data;
	adc_ops *ops;
	elec_dev_t *dev;
	int ret;

	if (!thiz) {
		return -1;
	}
	priv = ELEC_DRV(thiz)->private;

	ops = ELEC_DRV(priv->adc_mod)->ops;

	if (type >= ELECMAX)
		return -1;

	dev = &((elec_dev_t *)priv->dev)[type];

	raw_data = ops->adc_vol_val(priv->adc_mod, dev->adc_chn);

	ret = ((dev->cali_para / 1000.0) * (dev->idel_k ) * (raw_data - dev->zero_cali_para) + (dev->idel_b)) / 1000.0;

	return ret;
}

static int cali(void *thiz, int type)
{
	int i;
	int sum = 0;
	elec_private_t * priv;
	adc_ops *ops;
	elec_dev_t *dev;

	if (!thiz) {
		return -1;
	}
	priv = ELEC_DRV(thiz)->private;

	ops = ELEC_DRV(priv->adc_mod)->ops;

	if (type >= ELECMAX)
		return -1;

	dev = &((elec_dev_t *)priv->dev)[type];

	for (i = 0; i < 100; i++) {
		sum  += ops->adc_vol_val(priv->adc_mod, dev->adc_chn);
	}

	dev->zero_cali_para = sum / 100;

	return 0;
}

static int set_cali_para(void *thiz, int type, int para)
{
	elec_private_t * priv;
	adc_ops *ops;
	elec_dev_t *dev;

	if (!thiz) {
		return -1;
	}
	priv = ELEC_DRV(thiz)->private;

	ops = ELEC_DRV(priv->adc_mod)->ops;

	if (type >= ELECMAX)
		return -1;

	dev = &((elec_dev_t *)priv->dev)[type];
	dev->cali_para = para;

	return 0;
}

static elec_ops_t ops = {
	.init = init,
	.read = read_elec,
	.auto_cali = cali,
	.set_para = set_cali_para,
};

static int elec_module_register(void)
{
	elec_priv.dev_num = sizeof(elec_dev) / sizeof(elec_dev[0]);
	elec_mod.private = (void *)&elec_priv;
	elec_mod.ops = (void *)&ops;

	module_register(ELEC_MODULE, &elec_mod);

	return 0;
}

device_initcall(elec_module_register);

