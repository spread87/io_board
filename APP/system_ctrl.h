#ifndef __SYSTEM_CTRL_H__
#define __SYSTEM_CTRL_H__

#include "stm32f1xx.h"
#define SYSTEM_VER	0x0002

#define BIT(x)	(1 << (x))

typedef struct  {
	int init_done;
	int refresh_contrl;
	struct {
		uint16_t *lll_error_code;
		uint16_t *ll_error_code;
		uint16_t *l_error_code;
		uint16_t *c_error_flag;
		uint16_t *io_status;
	} error;
	struct {
		uint16_t *temp;
		uint16_t *fsm_status;
		uint16_t *status;
		uint16_t *f_status;
		uint16_t *cur_io_status;
		uint16_t *ext_err;
	} status;
	struct {
		uint16_t control;
	} control;
}__attribute__ ((aligned (4))) system_status_data_t;

typedef struct {
	struct {
		uint16_t system_version;
	}system;
	struct {
		uint16_t DI1_mode;
		uint16_t DI2_mode;
		uint16_t DI3_mode;
		uint16_t DI4_mode;
	}DI;
	struct {
		uint16_t DO1_mode;
		uint16_t DO2_mode;
		uint16_t DO3_mode;
		uint16_t DO4_mode;
	}DO;
	struct {
		uint16_t timer;
	}current_filter;
	struct {
		uint16_t baud;
		uint16_t parity;
		uint16_t id;
		uint16_t lantency;
	}com;
}__attribute__ ((aligned (4))) system_parameter_data_t;

typedef struct {
	int sync_flag;
	int sys_status;
	int full_vol;
	int recv_vol;
}sys_sync_t;

void system_initial(void);

#endif
