#ifndef __REG_H__
#define __REG_H__

#include "./mb.h" 
typedef struct {
	uint16_t max_len;
	uint16_t ms_map_addr;
	void * hold_reg_data;
	int (*process) (void *hold_st, uint8_t func_code,uint16_t *data, uint16_t *len);
	int (*period_task) (void *hold_st);
}hold_reg_t;


#define ROLE_MASTER				1
#define ROLE_SLAVE				2

#define REG_SYNC				0
#define REG_NOSYNC				1

#define MB_HOLD_READ			0
#define MB_HOLD_WRITE			1

#define HOLD_REGISTERS_NUMBER	32

#define SUPPORT_HOLD_REG_SLAVE 
//#define SUPPORT_HOLD_REG_MASTER
//
typedef struct {
	uint16_t reg;
}mb_reg_t;

#define HOLD_REG_WR(r, v) \

typedef struct {
	unsigned char device_id;
	unsigned char func_num;
	unsigned short addr;
	unsigned short reg_num;
}__attribute ((packed)) slave_hold_reg_head_t;

typedef struct{
	slave_hold_reg_head_t hold_head;
	unsigned char reg_byte_num;
	unsigned short data[0];
}__attribute ((packed)) slave_hold_reg_pdu_t;

typedef struct{
	slave_hold_reg_head_t hold_head;
	uint16_t crc;
}__attribute ((packed)) slave_hold_reg_ack_t;

typedef struct {
	unsigned char device_id;
	unsigned char func_num;
	unsigned char ret_len;
	unsigned short data[0];
}__attribute ((packed)) slave_hold_read_t;

typedef struct {
	unsigned char device_id;
	unsigned char func_num;
	unsigned char ret_len;
	unsigned short data[0];
}__attribute ((packed)) master_hold_read_t;

typedef struct {
	unsigned char device_id;
	unsigned char func_num;
	unsigned short addr;
	unsigned short data[0];
}__attribute ((packed)) slave_hold_06_reg_t;

typedef uint16_t (*reg_func) (uint16_t , uint8_t);

int hold_data_struct_length(void);
int hold_data_zone_init(void * data_p, uint16_t len);

int hold_driving_upload_proc(void *hold_st, uint8_t *data, uint16_t *ret_len, uint16_t addr, uint16_t addr_len);
void * hold_reg_get_description_st(uint8_t role);
int sci_hold_reg_callRegister(uint8_t role, uint16_t reg_num, reg_func callback);
int hold_process_main(void *hold_st, uint8_t func_code, uint16_t *data, uint16_t *len);
int hold_period_read_task(void *hold_st, uint8_t *data, uint16_t *len);
int sci_set_hold_reg_len(void *hold_st, uint16_t len);
int sci_holdReg_MS_mapping(uint16_t sla_addr);
int modbus_driving_tigger_event(uint16_t addr, uint16_t len);
int set_master_reg_value(uint16_t addr, uint16_t value);
uint16_t * hold_description_sigle_addr(void *regPool, void *in_data, int perimission);
#endif 
