#ifndef __MB_H__
#define __MB_H__

#include "cmsis_os.h"
#include "modbus_lib.h"
#include <stdio.h>
#include "stm32f1xx.h"

#include "./reg.h"

typedef struct{
	unsigned char device_id;
	unsigned char function_id;
	unsigned char error_id;
	unsigned short crc;
}__attribute ((packed)) error_code_t;

enum modbus_error_code {
	illegal_function = 0x01,
	illegal_address = 0x02,
	illegal_data = 0x03,
	illegal_crc = 0x08
};

enum opCode_t {
	WRITE = 0,
	READ,
};

#define MB_IDEL							0
#define MB_INIT_DONE					1
#define HOLD_REG_NUMBER					128

/* defien modbus type */
#define MB_TYPE_HOLD					1
#define MB_TYPE_CION					2

#define MDOBUS_SUPPORT_HOLD				(1 << 8)
#define MODBUS_SUPPORT_CION				(1 << 7)

/* define registers permission */
#define MB_PERMISSION_RO				1
#define MB_PERMISSION_VOLATILE			2

#define ERROR_LEN						sizeof(error_code_t)

QueueHandle_t driving_mailbox;

#define BD_DRIVING_TYPE_HOLD_PERIO		1
#define BD_DRIVING_TYPE_HOLD_MSG		2
#define BD_DRIVING_TYPE_HOLD_TIG		3
#define BD_DRIVING_TYPE_CIOn_PERIO		4
#define BD_DRIVING_TYPE_CIOn_MSG		5
#define BD_DRIVING_TYPE_CIOn_TIG		6

struct mb_list_item {
	uint16_t addr;
	uint16_t len;
	void *data;
	struct mb_list_item *next;
};

typedef struct mb_list_item mb_list_item_t;

typedef struct {
	uint16_t seg_num;
	int mb_type;
	mb_list_item_t *item;
}mb_list_t;

typedef struct {
	uint8_t type;
	uint8_t used;
	union {
		uint16_t counter;
		/* define hold reg message structure */
		struct {
			uint16_t reg_addr;
			uint16_t reg_len;
		};
	};
}driving_msg_t;

typedef struct {
	uint8_t mb_status;
	uint8_t role; /*master slave*/
	uint8_t id;
	void * com;
	void * hold;
	void * cion;
	void (*period_task) (void *arg);
	mb_list_t list;
}mb_com_pcb_t;


#define DRIV_MSG_TYPE(msg)	\
	(((driving_msg_t *)msg)->type)

static inline uint16_t htons(uint16_t n)
{
	  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

unsigned char * mb_error(unsigned char error, unsigned char id, unsigned char func);

int master_data_send(uint8_t *data, uint16_t len);
int slave_data_send(uint8_t *data, uint16_t len);
int driving_msg_send(void *msg);
void modbus_initial(int init_baud);
int slave_id_func_set(uint8_t id, uint8_t func_flag);
int master_id_func_set(uint8_t id, uint8_t func_flag);
int mb_slave_reg_ComBlock_register(int type, uint16_t addr, uint16_t len);
int mb_slave_description_sigle_addr(int type, uint16_t addr, uint16_t **data, void *ops, int perimission);
mb_list_item_t *mb_slave_get_item_by_segment(int type, uint16_t start, uint16_t end);
mb_list_item_t * mb_slave_get_item_by_addr(int type, uint16_t val);
#endif
