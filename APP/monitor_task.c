#include "system_ctrl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "thermal_bsp.h"
#include "cur_vol_bsp.h"
#include "module.h"
#include "gpio_bsp.h"
#include "system_ctrl.h"
#include "eeprom_26l64.h"

static module_t * thermal_inst;
static module_t * eeprom_inst;
extern system_status_data_t sys_info;
extern system_parameter_data_t sys_parameter;
sys_sync_t sys_sync;

static module_t *gpio_inst;
static struct GPIO_ops *gpio_ops;
volatile uint16_t current_di_val = 0;

static void io_status_update(uint16_t *status)
{
	uint16_t temp = 0;
#define DO_VALID(x) \
	(!!gpio_ops->read(gpio_inst, (x)))

#define DI_VALID(x)	(!DO_VALID(x))

	/*DI status*/
	temp = DI_VALID(P_DI0) << 0;
	temp |= DI_VALID(P_DI1) << 1;
	temp |= DI_VALID(P_DI2) << 2;
	temp |= DI_VALID(P_DI3) << 3;

	/*DO status */
	temp |= DO_VALID(P_DO0) << 8;
	temp |= DO_VALID(P_DO1) << 9;
	temp |= DO_VALID(P_DO2) << 10;
	temp |= DO_VALID(P_DO3) << 11;

	*status = temp;
}

#define DI_MODE_INVALID		0x00
#define DI_MODE_EX_ERROT	0x01

static int di_mode_select(int mode)
{
	int ret = DI_MODE_INVALID;
	switch (mode) {
		case 0:
			ret = DI_MODE_INVALID;
			break;
		case 5:
			ret = DI_MODE_EX_ERROT;
			break;
		default:
			break;
	}

	return ret;
}

extern void update_error_zone(void);

static void system_monitor_thread(const void *arg)
{
	thm_ops_t *thm_ops;
	volatile uint16_t temp1;
	volatile uint16_t temp_io = 0;
	volatile uint16_t f_io = 0;

	(void)arg;

	thermal_inst = module_get(0, "THERMAL");
	if (!thermal_inst) {
		while(1);
	}
	
	thm_ops = thermal_inst->ops;

	gpio_inst = module_get(0, "gpio");
	if (!gpio_inst)
		while(1);
	gpio_ops = gpio_inst->ops;

	gpio_ops->init(gpio_inst);

	thm_ops->init(thermal_inst);

	for(;;) {
		/* step 1: refersh system information */
		if (sys_info.init_done) {
			temp1 = (uint16_t)thm_ops->read_temp(thermal_inst, TEMP1);
			/* update io status */
			io_status_update(sys_info.status.cur_io_status);

		}

		/*DI0*/
		temp_io = *sys_info.status.cur_io_status;
		if (di_mode_select(sys_parameter.DI.DI1_mode) == DI_MODE_INVALID) {
			temp_io &= ~(1 << 0);
		}
		/*DI1*/
		if (di_mode_select(sys_parameter.DI.DI2_mode) == DI_MODE_INVALID) {
			temp_io &= ~(1 << 1);
		}
		/*DI2*/
		if (di_mode_select(sys_parameter.DI.DI3_mode) == DI_MODE_INVALID) {
			temp_io &= ~(1 << 2);
		}

		/*DI3*/
		if (di_mode_select(sys_parameter.DI.DI4_mode) == DI_MODE_INVALID) {
			temp_io &= ~(1<<3);
		}

		current_di_val = temp_io;
		*sys_info.status.status = current_di_val;
		*sys_info.status.ext_err = current_di_val & 0xF;

		/* error status of input */
		if (*sys_info.status.ext_err & (~f_io)) {
			/* update error information */
			*sys_info.error.io_status = *sys_info.status.cur_io_status;
//			*sys_info.error.lll_error_code = *sys_info.error.ll_error_code;
			*sys_info.error.ll_error_code = *sys_info.error.l_error_code;
			*sys_info.error.l_error_code = *sys_info.error.c_error_flag;
			*sys_info.error.c_error_flag = *sys_info.status.ext_err;
			taskENTER_CRITICAL();
			{
				update_error_zone();
			}
			taskEXIT_CRITICAL();
		}
		f_io = *sys_info.status.ext_err;
		/* save parameter */
		osDelay(5);
	}
}

PRIVILEGED_DATA static TaskHandle_t  system_monitor_handler= NULL;
void monitor_task_init(void)
{
	/* create system idel thread */
	osThreadDef(monitor_task, system_monitor_thread, osPriorityNormal, 0, 128);
	system_monitor_handler = osThreadCreate(osThread(monitor_task), NULL);
}
