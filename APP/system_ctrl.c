#include "system_ctrl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "module.h"
#include "modbus_hal.h"
#include "modbus_lib.h"
#include "timers.h"
#include "sci.h"
#include "gpio_bsp.h"
#include "adc_bsp.h"
#include "mb.h"

extern void sys_communication_initial(void);
static TaskHandle_t idel_hander;


static module_t *sys_led_inst;
static module_t *adc_inst;
#define SEND_DATA "12345678"
static void period_flash_led(TimerHandle_t xTimer)
{
	static int gpio_status = 0;
	struct GPIO_ops *ops;

	(void) xTimer;

	if (sys_led_inst == NULL)
		return;


	ops = sys_led_inst->ops;

	/* display system current status */
	gpio_status = !gpio_status;
	ops->write(sys_led_inst, P_LED_1_CTRL, &gpio_status);
//	slave_data_send((uint8_t *)SEND_DATA, sizeof(SEND_DATA));
}

PRIVILEGED_DATA static TaskHandle_t  system_idel_handler= NULL;
extern void monitor_task_init(void);
extern void control_task_init(void);
static void system_idel_thread(const void *arg)
{
	TimerHandle_t led_timer_handler;
	
	(void)arg;
	/* get system led */
	sys_led_inst = module_get(0, "gpio");
	if (sys_led_inst == NULL) {
		sys_log_e("obtain gpio module fail \n");
		while(1);
	}
	
	/* obtain idel task handler */
	idel_hander = xTaskGetHandle("IDLE");

	if (idel_hander == NULL) {
		sys_log_e("Don't found idel task\n");
		while(1);
	}

	adc_inst = module_get(0, "adc");
	if (adc_inst == NULL) {
		sys_log_e("Don't found idel task\n");
		while(1);
	}


	monitor_task_init();
	control_task_init();
	/* 100 ms refersh system status */
	led_timer_handler = xTimerCreate("ledTimer",
			400,
			pdTRUE,
			0,
			period_flash_led );

	xTimerStart(led_timer_handler, 0);
	for(;;) {
		osDelay(100);
	}
}

void system_initial(void)
{
	/* hardware initial */
	system_module_init();
	all_module_register();
	sys_communication_initial();
	mdelay(100);

	/* create system idel thread */
	osThreadDef(system_idel_task, system_idel_thread, osPriorityNormal, 0, 128);
	system_idel_handler = osThreadCreate(osThread(system_idel_task), NULL);
}
