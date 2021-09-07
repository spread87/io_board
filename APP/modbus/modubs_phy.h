#ifndef __MODBUS_PHY_H__
#define __MODBUS_PHY_H__

#include "cmsis_os.h"
#include "module.h"
#include "sci.h"

#define MODBUS_RECV_BUF_LEN		128
#define MODBUS_MAX_ID_NUM		2

typedef struct{
	uint32_t cur_ptr;
	uint8_t data[MODBUS_RECV_BUF_LEN];
	uint8_t flag;
}modbus_data_t;

typedef  struct {
	int (*write) (const void *thiz, uint8_t *, uint16_t);
	int (*read) (const void *thiz, uint8_t *, uint16_t);
}modbus_ops_t;

typedef struct {
	int id;
	int (*id_func)(void *modbus_phy, uint8_t *data, uint16_t len);
	void *param;
}device_id_t;

typedef struct {
	uint32_t mb_if_num;
	module_t * com_if;
	module_t * timer_if;
	uint32_t timer_val;
	char * name;
	modbus_data_t data;
	modbus_ops_t * ops;

	int current_device_id_num;
	device_id_t device_id[MODBUS_MAX_ID_NUM];
}modbus_phy_t;

enum MODBUS_IF_ID{
	MODBUS_IF_0 = 0,
	MODBUS_IF_1,
	MODBUS_IF_MAX
};


/* queue */
#define MODBUS_MAILBOX_LEN	10
QueueHandle_t modbus_maxbox;

/* declare */
void modbus_phy_init(int init_baud);

int modbus_id_binding(int if_num, uint8_t id,
	int (*id_func)(void *, uint8_t *, uint16_t),
	void *para, void **com);
#endif

