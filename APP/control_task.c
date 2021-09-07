#include "system_ctrl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "thermal_bsp.h"
#include "cur_vol_bsp.h"
#include "module.h"
#include "gpio_bsp.h"
#include "cur_vol_bsp.h"
#include "sci.h"

extern sys_sync_t sys_sync;

enum FSM_e{
	FSM_SYSTEM_INIT = 0,
	FSM_NORMAL, 
};

enum CTRL_e{
	CLOSE,
	OPEN
};

static int current_fsm_status = FSM_SYSTEM_INIT;
static int next_fsm_status = FSM_SYSTEM_INIT;

extern system_status_data_t sys_info;
extern system_parameter_data_t sys_parameter;
extern volatile uint16_t current_di_val;

#define IsNormalFSM(f)	(f == FSM_NORMAL)
#define ISErrorFSM(f) 	(f == FSM_SYSTEM_INIT)

#define DO_MODE_INVALID		0x00
#define DO_MODE_COM_CTRL	0x01
#define DO_MODE_DI_CTRL		0x02

static int DO_function(int val)
{
	int ret = DO_MODE_INVALID;
	switch (val) {
		case 0:
			ret = DO_MODE_INVALID;
			break;
		case 1:
			ret = DO_MODE_COM_CTRL;
			break;
		case 5:
			ret = DO_MODE_DI_CTRL;
			break;
		default:
			return ~0;
	}

	return ret;
}


static int control_status_fsm(int type, int extern_st)
{
	(void) type;
	(void) extern_st;

	switch(current_fsm_status) {
		case FSM_SYSTEM_INIT:
				next_fsm_status = FSM_NORMAL;
			break;
		case FSM_NORMAL:
			break;
	}
	return 0;
}

static module_t *gpio_inst;
static struct GPIO_ops * gpio_ops;

static module_t * elec_inst;

int last_error_fsm = FSM_SYSTEM_INIT;

static void DO_ctrl(uint16_t do_mode, int io)
{
#define DO_VALID(x) \
	(!!gpio_ops->read(gpio_inst, (x)))

#define DI_VALID(x)	(!DO_VALID(x))

	int temp = 0;
	switch(DO_function(do_mode)) {
		case DO_MODE_COM_CTRL:
			if (sys_info.control.control & (1 << (io - P_DO0))) {
				temp = 1;
				gpio_ops->write(gpio_inst, io, &temp);
			} else {
				temp = 0;
				gpio_ops->write(gpio_inst, io, &temp);
			}

			break;
		case DO_MODE_DI_CTRL:
			if (current_di_val & (1 << (io - P_DO0)))	 {
				temp = 1;
				gpio_ops->write(gpio_inst, io, &temp);
			} else {
				temp = 0;
				gpio_ops->write(gpio_inst, io, &temp);
			}
			break;
		default:
			temp = 0;
			gpio_ops->write(gpio_inst, io, &temp);
			break;
	}
}


static void system_ctrl_thread(const void *arg)
{
	elec_ops_t *elec_ops;
	(void) arg;

	elec_inst = module_get(0, "ELECTRON");
	if (!elec_inst) {
		while(1);
	}

	elec_ops = elec_inst->ops;

	/* power on seq*/
	gpio_inst = module_get(0, "gpio");
	if (!gpio_inst)
		while(1);
	gpio_ops = gpio_inst->ops;

	gpio_ops->init(gpio_inst);
	/* initial gpio status */
	for (;;) { 
		taskENTER_CRITICAL();
		{
			if (sys_sync.sync_flag) {
				control_status_fsm(sys_sync.sys_status, 0);
				sys_sync.sync_flag = 0;
				sys_sync.sys_status = 0;
				}
		}
		taskEXIT_CRITICAL();
		control_status_fsm(0,0);
// for debug
		*sys_info.status.fsm_status = current_fsm_status;

		switch(current_fsm_status) {
			case FSM_SYSTEM_INIT:
				if (next_fsm_status != FSM_SYSTEM_INIT) {
					;
				}
				break;
			case FSM_NORMAL:
				/*DO*/
				DO_ctrl(sys_parameter.DO.DO1_mode, P_DO0);
				DO_ctrl(sys_parameter.DO.DO2_mode, P_DO1);
				DO_ctrl(sys_parameter.DO.DO3_mode, P_DO2);
				DO_ctrl(sys_parameter.DO.DO4_mode, P_DO3);
				break;
			default:
				while(1);
				break;
		}

		if (IsNormalFSM(current_fsm_status) && ISErrorFSM(next_fsm_status)) {
			taskENTER_CRITICAL();
			{
				last_error_fsm = next_fsm_status;
			}
			taskEXIT_CRITICAL();
		}

		current_fsm_status = next_fsm_status;
		osDelay(5);
	}
}

PRIVILEGED_DATA static TaskHandle_t  system_ctrl_handler= NULL;
void control_task_init(void)
{
	/* create system idel thread */
	osThreadDef(ctrl_task, system_ctrl_thread, osPriorityNormal, 0, 256);
	system_ctrl_handler = osThreadCreate(osThread(ctrl_task), NULL);
}
