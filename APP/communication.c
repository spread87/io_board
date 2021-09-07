#include "mb.h"
#include "modbus_hal.h"
#include "modbus_lib.h"
#include "module.h"
#include "gpio_bsp.h"
#include "sci.h"
#include "system_ctrl.h"
#include "string.h"
#include "eeprom_26l64.h"

system_status_data_t sys_info = {
	.init_done = 0,	
};

system_parameter_data_t factory_sys_parameter = {
	.system.system_version = SYSTEM_VER,
	.DI = {
		.DI1_mode = 0,
		.DI2_mode = 0,
		.DI3_mode = 0,
		.DI4_mode = 0,
	},
	.DO = {
		.DO1_mode = 1,
		.DO2_mode = 1,
		.DO3_mode = 1,
		.DO4_mode = 1,
	},
	.current_filter = {
		.timer = 5888,
	},
	.com = {
		.baud = 7,
		.parity = 0,
		.id = 3,
		.lantency = 0,
	},
};

enum {
	P_HEAD = 0,
	P_CONTROL,
	P_ERROR_ST,
};


extern void reset_control_zone(void);
static int write_para(int type, uint16_t st_addr, uint16_t len, uint8_t * data);
static void reset_error_zone(void);

system_parameter_data_t sys_parameter;


/* reset configure parameter */
static uint16_t _reg_0000(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		if (val == 4) {
			reset_control_zone();
			reset_error_zone();
		}
	}

	return 0;
}

/* DI0 configure parameter */
static uint16_t _reg_0500(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DI.DI1_mode = val;
		
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DI.DI1_mode), 
				2, (uint8_t *)&val);

	} else if (type == MB_HOLD_READ){
		return sys_parameter.DI.DI1_mode;
	}

	return 0;
}

/* DI1 configure parameter */
static uint16_t _reg_0501(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DI.DI2_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DI.DI2_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DI.DI2_mode;
	}

	return 0;
}

static uint16_t _reg_0502(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DI.DI3_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DI.DI3_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DI.DI3_mode;
	}

	return 0;
}

static uint16_t _reg_0503(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DI.DI4_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DI.DI4_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DI.DI4_mode;
	}

	return 0;
}


/* DO0 configure parameter */
static uint16_t _reg_0600(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DO.DO1_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DO.DO1_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DO.DO1_mode;
	}

	return 0;
}

/* DO0 configure parameter */
static uint16_t _reg_0601(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DO.DO2_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DO.DO2_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DO.DO2_mode;
	}

	return 0;
}

static uint16_t _reg_0602(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DO.DO3_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DO.DO3_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DO.DO3_mode;
	}

	return 0;
}

static uint16_t _reg_0603(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.DO.DO4_mode = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->DO.DO4_mode), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.DO.DO4_mode;
	}

	return 0;
}


/* current filter timer configure parameter */
static uint16_t _reg_0802(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.current_filter.timer = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->current_filter.timer), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.current_filter.timer;
	}

	return 0;
}



/* battery number configure parameter */
static uint16_t _reg_0d00(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.com.baud = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->com.baud), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.com.baud;
	}

	return 0;
}

/* battery number configure parameter */
static uint16_t _reg_0d01(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.com.parity = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->com.parity), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.com.parity;
	}

	return 0;
}

/* battery number configure parameter */
static uint16_t _reg_0d02(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.com.id = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->com.id), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.com.id;
	}

	return 0;
}

/* battery number configure parameter */
static uint16_t _reg_0d04(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_parameter.com.lantency = val;
		write_para(P_CONTROL, 
				(uint16_t)&(((system_parameter_data_t *)0)->com.lantency), 
				2, (uint8_t *)&val);
	} else if (type == MB_HOLD_READ){
		return sys_parameter.com.lantency;
	}

	return 0;
}
static uint16_t hide_para_magic = 0;
/* battery number configure parameter */
static uint16_t _reg_0f00(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		if (val == 0x0313) {
			hide_para_magic = 1;
		}
	} else if (type == MB_HOLD_READ){
		if (!hide_para_magic)
			return 0;
		else
			return 0x0313;
	}

	return 0;
}

/* software verison parameter */
static uint16_t _reg_1101(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		return 0;
	} else if (type == MB_HOLD_READ){
		return factory_sys_parameter.system.system_version;
	}

	return 0;
} 

static uint16_t _reg_8001(int type, uint16_t val)
{
	(void)val;
	if (type == MB_HOLD_WRITE) {
		sys_info.control.control = val;
		return 0;
	} else if (type == MB_HOLD_READ){
		return sys_info.control.control;
	}

	return 0;
} 


void reset_parameter(void)
{
	memcpy(&sys_parameter, &factory_sys_parameter, sizeof(system_parameter_data_t));
}

static int ret_baud(int type)
{
	int baud;

	switch (type){
		case 4:
			baud = 4800;
			break;
		case 5:
			baud = 9600;
			break;
		case 6:
			baud = 19200;
			break;
		case 7:
			baud = 38400;
			break;
		case 8:
			baud = 57600;
			break;
		case 9:
			baud = 115200;
			break;
		default:
			baud = 115200;
			break;
	}

	return baud;
}

#define EPM_INIT_OK		0
#define EPM_INIT_FAIL	1
#define EPM_READ_FAIL	2
#define EPM_WRITE_FAIL	3
static module_t *eeprom_inst;
static eeprom_ops_t *e_ops;
static int eeprom_valid = EPM_INIT_FAIL;
#define HEAD_STR	"para valid!"

static uint32_t get_para_addr(int type)
{
	switch(type){
		case P_HEAD:
			return PARA_HEAD_ADDR;
		case P_CONTROL:
			return PARA_0_ADDR;
		case P_ERROR_ST:
			return PARA_1_ADDR;
		default:
			return ~0;
	}
}

static int read_para(int type, uint16_t len, uint8_t *data);
uint8_t write_cali_buf[32];

static int write_para(int type, uint16_t st_addr, uint16_t len, uint8_t * data)
{
	uint32_t addr = get_para_addr(type);
	volatile uint32_t i;
	volatile uint32_t loop_cnt = 0;

	if (!data || !len)
		return -1;


	addr += st_addr;
WRITE_LOOP:

	for (i = 0; i < len; i++)
	{
		if (e_ops->write(eeprom_inst, addr+i, data[i])) {
			if (loop_cnt ++ < 5)
				goto WRITE_LOOP;
			return -1;
		}
		udelay(5500);
	}

#if 0
	crc = usMBCRC16(data, len);

	if (e_ops->write(eeprom_inst, addr + len + 0, crc & 0xFF)) {
			if (loop_cnt ++ < 5)
				goto WRITE_LOOP;
		return -1;
	}

	if (e_ops->write(eeprom_inst, addr + len + 1, crc>>8)) {
			if (loop_cnt ++ < 5)
				goto WRITE_LOOP;
		return -1;
	}

	/* write check */
	read_para(type, len + 2, write_cali_buf);

	crc = usMBCRC16(write_cali_buf, len);

	if (crc != *((uint16_t *)&write_cali_buf[len]))
		if (loop_cnt ++ < 5)
			goto WRITE_LOOP;

#endif
	return 0;
}

#define PARA_ERROR	1
#define DEVICE_ERROR	2
#define CRC_ERROR	3

static int read_para(int type, uint16_t len, uint8_t *data)
{
	uint32_t addr = get_para_addr(type);
	volatile uint32_t i;
	volatile uint32_t loop_cnt = 0;

	if (!data || !len)
		return PARA_ERROR;
	
READ_LOOP:
	for (i = 0; i < len; i++) {
		udelay(100);
		if (e_ops->read(eeprom_inst, addr + i, 1, data + i)) {
			if (loop_cnt ++ < 5)
				goto READ_LOOP;
			return DEVICE_ERROR;
		}
	}

//	calc_crc = usMBCRC16(data, len);
//
//	if (e_ops->read(eeprom_inst, addr + len, 2, (uint8_t *)&read_crc)) {
//		if (loop_cnt ++ < 5)
//			goto READ_LOOP;
//		return DEVICE_ERROR;
//	}
//
//	if (calc_crc != read_crc) {
//		if (loop_cnt ++ < 5)
//			goto READ_LOOP;
//		return CRC_ERROR;
//	}
	
	return 0;
}

static uint16_t eeprom_step = 0;

void runtimer_info(void)
{
	char head[16];
	char * orgn = HEAD_STR;
	int err_n;
	int n = 1;
	if (!eeprom_inst)
		while(1);

	e_ops = eeprom_inst->ops;
	e_ops->init(eeprom_inst);

	/* runtime information save */
	eeprom_step = 1;
	while (n) {
		err_n = read_para(P_HEAD, sizeof(HEAD_STR), (uint8_t *)head);
		if (err_n == DEVICE_ERROR)
		{
			if (n++ < 5)
				continue;
			eeprom_valid = EPM_INIT_FAIL;
			goto ERROR;
		}
		break;
	}

	eeprom_step = 2;
	if (strcmp(orgn, head)) {
		reset_parameter();
		/* update information */
		if (write_para(P_HEAD, 0, sizeof(HEAD_STR), (uint8_t*)orgn)) {
			eeprom_valid = EPM_WRITE_FAIL;
			return ;
		}
		/* update control information */
		if (write_para(P_CONTROL, 0, sizeof(factory_sys_parameter), (uint8_t *)&factory_sys_parameter)) {
			eeprom_valid = EPM_WRITE_FAIL;
			return ;
		}
		return ;
	}
	/* recover control information */

	eeprom_step = 3;
	if (read_para(P_CONTROL, sizeof(sys_parameter), (uint8_t *)&sys_parameter)) {
		eeprom_valid = EPM_READ_FAIL;
		goto ERROR;
		return ;
	}

	eeprom_valid = EPM_INIT_OK;
	return ;
ERROR:
	reset_parameter();
	return ;
}

void reset_control_zone(void)
{
	uint32_t i ;
	/* system refersh fail */

	/* update control information */
	for (i = 0; i < 5; i++) {
		if (write_para(P_CONTROL, 0, sizeof(factory_sys_parameter), (uint8_t *)&factory_sys_parameter))
			continue;
		break;
	}

}

void update_control_zone(void)
{
	uint32_t i ;
	/* system refersh fail */
	if (eeprom_valid)
		return ;

	/* update control information */
	for (i = 0; i < 5; i++) {
		if (write_para(P_CONTROL, 0, sizeof(sys_parameter), (uint8_t *)&sys_parameter))
			continue;
		break;
	}
}

static void reset_error_zone(void)
{
	uint32_t i;
	uint16_t data[9];

	if (eeprom_valid)
		return ;

	i = 0;

	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;
	data[i++] = 0;

	for (i = 0; i < 5; i++) {
		if (write_para(P_ERROR_ST, 0, sizeof(data), (uint8_t *)&data))
			continue;
		break;
	}

}

void update_error_zone(void)
{
	uint32_t i;
	uint16_t data[4];

	if (eeprom_valid)
		return ;

	i = 0;

	data[i++] = *sys_info.error.ll_error_code;
	data[i++] = *sys_info.error.l_error_code;
	data[i++] = *sys_info.error.c_error_flag;
	data[i++] = *sys_info.error.io_status;

	for (i = 0; i < 5; i++) {
		if (write_para(P_ERROR_ST, 0, sizeof(data), (uint8_t *)&data))
			continue;
		break;
	}
}

void error_code_init(void)
{
	uint32_t i;
	uint16_t data[4];

	if (eeprom_valid)
		return ;

	for (i = 0; i < 5; i++) {
		if (read_para(P_ERROR_ST, sizeof(data), (uint8_t *)&data))
			continue;
		break;
	}


	i = 0;

//	*sys_info.error.lll_error_code = data[i++];
	*sys_info.error.ll_error_code = data[i++];
	*sys_info.error.l_error_code = data[i++];
	*sys_info.error.c_error_flag = data[i++];
	*sys_info.error.io_status = data[i++];

//	*sys_info.error.lll_error_code = *sys_info.error.ll_error_code;
//	*sys_info.error.ll_error_code = *sys_info.error.l_error_code;
//	*sys_info.error.l_error_code = *sys_info.error.c_error_flag;
}


void sys_communication_initial(void)
{
	/* initial modbus structure*/

	eeprom_inst = module_get(0, "eeprom");
	if (!eeprom_inst)
		while(1);

	runtimer_info();

	modbus_initial(ret_baud(sys_parameter.com.baud));
	slave_id_func_set(sys_parameter.com.id, (uint8_t)MDOBUS_SUPPORT_HOLD);

	/* register slave modebus */
	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x00, 16);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x00, NULL, _reg_0000, MB_PERMISSION_VOLATILE);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x500, 10);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x500, NULL, _reg_0500, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x501, NULL, _reg_0501, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x502, NULL, _reg_0502, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x503, NULL, _reg_0503, MB_PERMISSION_VOLATILE);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x600, 22);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x600, NULL, _reg_0600, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x601, NULL, _reg_0601, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x602, NULL, _reg_0602, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x603, NULL, _reg_0603, MB_PERMISSION_VOLATILE);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x800, 26);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x900, 22);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0xd00, 4);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0xd00, NULL, _reg_0d00, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0xd01, NULL, _reg_0d01, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0xd02, NULL, _reg_0d02, MB_PERMISSION_VOLATILE);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0xd04, NULL, _reg_0d04, MB_PERMISSION_VOLATILE);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x1000, 20);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1000, 
			&sys_info.error.ll_error_code,NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1001, 
			&sys_info.error.l_error_code,NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1002, 
			&sys_info.error.c_error_flag, NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1003, 
			&sys_info.error.io_status, NULL, MB_PERMISSION_RO);
	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x1100, 2);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1101,NULL, _reg_1101, MB_PERMISSION_VOLATILE);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x1200, 24);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1200, 
			&sys_info.status.cur_io_status,NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x1201, 
			&sys_info.status.ext_err,NULL, MB_PERMISSION_RO);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0xf00, 22);

	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x8000, 0x0d+1);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x8001,NULL, _reg_8001, MB_PERMISSION_VOLATILE);
	mb_slave_reg_ComBlock_register(MB_TYPE_HOLD, 0x810b, 0x08);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x810b, 
			&sys_info.status.f_status,NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x810c, 
			&sys_info.status.status,NULL, MB_PERMISSION_RO);
	mb_slave_description_sigle_addr(MB_TYPE_HOLD, 0x810d, 
			&sys_info.status.fsm_status,NULL, MB_PERMISSION_RO);

	sys_info.init_done = 1;

	error_code_init();
}

