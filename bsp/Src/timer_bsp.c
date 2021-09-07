#include "timer_bsp.h"
#include "string.h"
#include "sci.h"
#include "module.h"
#include "tim.h"
#include "gpio_bsp.h"

typedef struct tim_set_st{
	module_t tim_mod;
	timer_private_t tim_private;
}tim_set_t;

tim_set_t tim_set[] = {
	{
		.tim_mod.name = "tim2",
#define TIM_1_MOD_BASE		TIM2
		.tim_private.base = TIM_1_MOD_BASE,
	},
	{
		.tim_mod.name = "tim3",
#define TIM_2_MOD_BASE		TIM3
		.tim_private.base = TIM_2_MOD_BASE,
	},
	{
		.tim_mod.name = "tim4",
#define TIM_3_MOD_BASE		TIM4
		.tim_private.base = TIM_3_MOD_BASE,
	}

};

static void timer_if_init(void *thiz)
{
	module_t * tim_mod = (module_t *)thiz;
	timer_private_t * tim_pri;

	if (tim_mod == NULL) {
		sys_log_e("input invalid parameters\n");
		while(1);
	}

	tim_pri = (timer_private_t*)tim_mod->private;

	switch ((uint32_t)(tim_pri->base)) {
#if defined (TIM_1_MOD_BASE)
		case (uint32_t)TIM_1_MOD_BASE:
			MX_TIM2_Init();
		break;
#endif
#if defined (TIM_2_MOD_BASE)
		case (uint32_t)TIM_2_MOD_BASE:
			MX_TIM3_Init();
		break;
#endif
#if defined (TIM_3_MOD_BASE)
		case (uint32_t)TIM_3_MOD_BASE:
			MX_TIM4_Init();
		break;
#endif
		default:
			sys_log_e("invalid base addr\n");
		break;
	}
}

static int timer_start(void *thiz)
{
	timer_private_t * tim_pri;
	module_t * tim_mod = (module_t *)thiz;

	if (tim_mod == NULL) {
		sys_log_e("input invalid parameter");
		return -1;
	}

	tim_pri = (timer_private_t *)tim_mod->private;

	tim_start(tim_pri->base);
	return 0;
}

static int timer_stop(void *thiz)
{
	timer_private_t * tim_pri;
	module_t * tim_mod = (module_t *)thiz;

	if (tim_mod == NULL) {
		sys_log_e("input invalid parameter");
		return -1;
	}

	tim_pri = (timer_private_t *) tim_mod->private;

	tim_stop(tim_pri->base);
	return 0;
}

static void tim_int_ctrl(void *thiz, uint32_t en)
{
	timer_private_t * tim_pri;
	module_t * tim_mod = (module_t *)thiz;

	if (tim_mod == NULL) {
		sys_log_e("input invalid parameter");
		while(1);
	}

	tim_pri = (timer_private_t *) tim_mod->private;
	tim_phy_int_en(tim_pri->base,  en);
}

void irq_tim_handler(void * tim_base)
{
	timer_private_t *tim_pri;
	if (tim_base == NULL)
		return ;

	switch((uint32_t)tim_base) {
#if defined (TIM_1_MOD_BASE)
		case (uint32_t)TIM_1_MOD_BASE:
			tim_pri = &tim_set[0].tim_private;
			tim_pri->call_back(tim_pri->fun_para);
		break;
#endif
#if defined (TIM_2_MOD_BASE)
		case (uint32_t)TIM_2_MOD_BASE:
			tim_pri = &tim_set[1].tim_private;
			tim_pri->call_back(tim_pri->fun_para);
		break;
#endif
#if defined (TIM_3_MOD_BASE)
		case (uint32_t)TIM_3_MOD_BASE:
			tim_pri = &tim_set[2].tim_private;
			tim_pri->call_back(tim_pri->fun_para);
		break;
#endif

		default:
		break;
	}
}

static int tim_register_callback(
		void *thiz,
		void (*tim_call)(void *arg),
		void * func_para
)
{
	module_t *tim_mod = (module_t *)thiz;
	timer_private_t * tim_pri;
	if (tim_mod == NULL) {
		return -1;
	}

	tim_pri = (timer_private_t *)tim_mod->private;
	
	tim_pri->call_back = tim_call;
	tim_pri->fun_para = func_para;
	return 0;
}

static void reload_timer_counter(void *thiz, uint32_t cnt, uint32_t force_en)
{
	timer_private_t *tim_pri;
	module_t * tim_mod = (module_t *)thiz;
	if (tim_mod == NULL) {
		sys_log_e("input invaid parameters\r\n");
		return ;
	}

	tim_pri = (timer_private_t *)tim_mod->private;

	reload_timer_counter_value(tim_pri->base, cnt, force_en);
}

static int  create_timer_event(void *thiz, uint32_t reload_val,
void (*call)(void *), void *arg, uint32_t start)
{
	module_t * tim_mod = (module_t *)thiz;
	timer_ops_t * ops;
	if (tim_mod == NULL) {
		sys_log_e("input invaid parameters\r\n");
		return -1;
	}

	ops = tim_mod->ops;

	/* stop timer */
	ops->stop(thiz);
	ops->reload(thiz, reload_val, 0);
	ops->register_call(thiz, call, arg);
	ops->enable_int(thiz, 1);

	if (start)
		ops->start(thiz);
	
	return 0;
}

static timer_ops_t timer_ops = {
	.init = timer_if_init,
	.start = timer_start,
	.stop = timer_stop,
	.register_call = tim_register_callback,
	.reload = reload_timer_counter,
	.create_timer = create_timer_event,
	.enable_int = tim_int_ctrl,
};

static int timer_module_register(void)
{
	static int init_flag = 0;
	int i;

	if (init_flag) {
		return 0;
	}

	for (i = 0; i < (int)(sizeof(tim_set) / sizeof(tim_set_t)); i++) {
		tim_set[i].tim_mod.ops = (void *)&timer_ops;
		tim_set[i].tim_mod.private = (void *)&tim_set[i].tim_private;
		module_register(TIM_1_MODULE + i, &tim_set[i].tim_mod);
	}
	init_flag++;
	return 0;
}

device_initcall(timer_module_register);


/*************************test*****************************/
module_t * GPIO_inst;
module_t * TIM_inst;
module_t * TIM1_inst;

void self_timer_test(void *arg)
{
	struct GPIO_ops *gpio_ops;
	static volatile int in_cnt = 0;
	static volatile int gpio_val = 1;

	timer_ops_t * tim_ops;
	tim_ops = TIM_inst->ops;

	(void)arg;
	gpio_ops = GPIO_inst->ops;

	if (in_cnt++ > 500) {
		gpio_val = !gpio_val;
		gpio_ops->write((void *)GPIO_inst, P_LED_2_CTRL, (void *)&gpio_val);
		in_cnt = 0;
	}

	tim_ops->create_timer((void *)TIM_inst,
			1000, self_timer_test, (void *)TIM_inst,1);
}

void self_timer_1_test(void *arg)
{
	struct GPIO_ops *gpio_ops;
	static volatile int in_cnt = 0;
	static volatile int gpio_val = 1;

	timer_ops_t * tim_ops;
	tim_ops = TIM1_inst->ops;

	gpio_ops = GPIO_inst->ops;
	(void)arg;

	if (in_cnt++ > 300) {
		gpio_val = !gpio_val;
		gpio_ops->write((void *)GPIO_inst, P_LED_1_CTRL, (void *)&gpio_val);
		in_cnt = 0;
	}

	tim_ops->create_timer((void *)TIM1_inst,
			1000, self_timer_1_test, (void *)TIM1_inst,1);
}

static void tim_self_test_thread_func(const void *arg)
{
	struct GPIO_ops *gpio_ops;
	timer_ops_t * tim_ops;
	timer_ops_t * tim1_ops;
//	int gpio_val = 0;

	(void) arg;

	GPIO_inst = module_get(0, "gpio");
	if (GPIO_inst == NULL) {
		sys_log_e("get gpio module fail!!!\n");
		while(1);
	}
	TIM_inst = module_get(0, "tim2");
	if (TIM_inst == NULL) {
		sys_log_e("get timer module fail!!!\n");
		while(1);
	}

	TIM1_inst = module_get(0, "tim4");
	if (TIM1_inst == NULL) {
		sys_log_e("get timer module fail!!!\n");
		while(1);
	}


	gpio_ops = GPIO_inst->ops;
	tim_ops = TIM_inst->ops;
	tim1_ops = TIM1_inst->ops;

	/* initial */
	gpio_ops->init((void *)GPIO_inst);
	tim_ops->init((void *)TIM_inst);
	tim1_ops->init((void *)TIM1_inst);

	tim_ops->stop((void *)TIM_inst);
	tim_ops->create_timer((void *)TIM_inst,
			1000, self_timer_test, (void *)TIM_inst,1);

//	tim1_ops->stop((void *)TIM1_inst);
//	tim1_ops->create_timer((void *)TIM1_inst,
//			1000, self_timer_1_test, (void *)TIM1_inst,1);
//
	while(1) {
//		gpio_val = 0;
//		gpio_ops->write((void *)GPIO_inst, P_LED_1_CTRL, &gpio_val);
//		osDelay(1000);
//		gpio_val = 1;
//		gpio_ops->write((void *)GPIO_inst, P_LED_1_CTRL, &gpio_val);
		osDelay(1000);
	}
}

PRIVILEGED_DATA static TaskHandle_t tim_self_test_handle = NULL;

void gpio_self_test_thread_init(void)
{
  return;
  osThreadDef(tim_self_test_task, tim_self_test_thread_func, osPriorityNormal, 0, 128);
  tim_self_test_handle = osThreadCreate(osThread(tim_self_test_task), NULL);
}


