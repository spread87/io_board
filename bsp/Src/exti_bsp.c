#include "exti_bsp.h"
#include "stm32f1xx.h"



static module_t exti_module = {
	.name = "exti",
};

struct exti_private_t  {
	EXTI_TypeDef * base;
	void * fun_para;
	uint32_t (* call_back) (void *arg, uint32_t exti_val);
};

static struct exti_private_t exti_private = {
#define EXTI_BASE_ADDR	EXTI
	.base = EXTI_BASE_ADDR,
};



static void exti_init(void *thiz)
{
	module_t * exti_m;
	struct exti_private_t * exti_pri;
	EXTI_TypeDef * exti_base;

	if (thiz == NULL) {
		sys_log_e("input invalid parameters\n");
		while(1);
	}

	exti_m = thiz;

	exti_pri = exti_m->private;
	exti_base = exti_pri->base;

	/* clear all interrupt flag */
	exti_base->PR = 0xfffff;
}

static int disable_exti(void *thiz, uint16_t n)
{
	module_t * exti_m;
	struct exti_private_t * exti_pri;

	if (thiz == NULL) {
		sys_log_e(" input Invalid parameter\n");
		return -1;
	}

	exti_m = (module_t *)thiz;
	exti_pri = (struct exti_private_t *)exti_m->private;
	exti_pri->base->IMR &= ~n;

	return 0;
}

static int diable_nvic_it(void *thiz, char flag)
{
	(void)thiz;


	if (flag) {
		HAL_NVIC_DisableIRQ(EXTI0_IRQn);
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
		HAL_NVIC_DisableIRQ(EXTI2_IRQn);
		HAL_NVIC_DisableIRQ(EXTI3_IRQn);
		HAL_NVIC_DisableIRQ(EXTI4_IRQn);
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	} else {
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);
		HAL_NVIC_EnableIRQ(EXTI2_IRQn);
		HAL_NVIC_EnableIRQ(EXTI3_IRQn);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	}

}

static int enable_exti(void *thiz, uint16_t n)
{
	module_t * exti_m;
	struct exti_private_t * exti_pri;

	if (thiz == NULL) {
		sys_log_e(" input Invalid parameter\n");
		return -1;
	}

	exti_m = (module_t *)thiz;
	exti_pri = (struct exti_private_t *)exti_m->private;
	/* clear interrupt flag */
	exti_pri->base->PR |= n;
	exti_pri->base->IMR |= n;

	return 0;
}

static uint32_t get_exti_value(void *thiz)
{
	module_t * exti_m;
	struct exti_private_t *exti_pri;

	if (thiz == NULL) {
		sys_log_e("input invalid parameter\n");
		return -1;
	}

	exti_m = (module_t *)thiz;

	exti_pri = exti_m->private;

	exti_pri->base->PR;
}

static int exti_callback_register( void *thiz,
	uint32_t (* call_back) (void *arg, uint32_t exti_val),
	void *arg)
{
	module_t * exti_m;
	struct exti_private_t *exti_pri;

	if (thiz == NULL) {
		sys_log_e("input invalid parameter\n");
		return -1;
	}

	exti_m = (module_t *)thiz;

	exti_pri = exti_m->private;

	exti_pri->call_back = call_back;
	exti_pri->fun_para = arg;

	return 0;
}

static int read_exti_val(void * thiz)
{
	module_t *exti_m;
	struct exti_private_t *exti_pri;
	
	if (thiz == NULL) {
		return -1;
	}

	exti_m = thiz;

	exti_pri = exti_m->private;

	return exti_pri->base->PR;
}

static int read_clear_val(void * thiz)
{
	int ret_val;
	module_t *exti_m;
	struct exti_private_t *exti_pri;

	if (thiz == NULL) {
		return -1;
	}

	exti_m = thiz;

	exti_pri = exti_m->private;

	ret_val = exti_pri->base->PR;

	exti_pri->base->PR = ret_val;

	return ret_val; 
}


void _exti_handler(void * exti_base)
{
	EXTI_TypeDef * p_exti_base;
	struct exti_private_t *exti_pri;
	uint32_t current_trig_val;
	uint32_t detail_val;

	p_exti_base = exti_base;

	if (p_exti_base == NULL)
		return;

	current_trig_val = p_exti_base->PR;

	switch((uint32_t) p_exti_base) {
		case (uint32_t)EXTI:
#if defined (EXTI_BASE_ADDR)
			exti_pri = exti_module.private;
			if (exti_pri->call_back != NULL) {
				detail_val |= (exti_pri->call_back(exti_pri->fun_para, current_trig_val));
			} else {
				detail_val = current_trig_val;
			}
			break;
#endif
		default:
			detail_val = current_trig_val;
			break;

	}

	p_exti_base->PR = detail_val;

}

static struct exti_ops_t exti_ops = {
	.init = exti_init,
	.disable_exti_it = disable_exti,
	.enable_exti_it = enable_exti,
	.call_back_register = exti_callback_register,
	.read_exti = read_exti_val,
	.read_clear_exti = read_clear_val,
	.close_exit_nvic_it = diable_nvic_it,
};

static int exti_module_init(void)
{
	exti_module.ops = (void *)&exti_ops;
	exti_module.private = (void *)&exti_private;
	module_register(EXTI_MODULE, &exti_module);
	return 0;
}


device_initcall(exti_module_init);

