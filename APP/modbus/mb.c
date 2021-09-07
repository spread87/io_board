#include "mb.h"
#include "modbus_hal.h"
#include "modubs_phy.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "timers.h"
#include "string.h"

#define mb_malloc	pvPortMalloc

static unsigned char error_buf[sizeof(error_code_t)];
static mb_list_t reg_list_head;
static mb_com_pcb_t master_com_pcb;
static mb_com_pcb_t slave_com_pcb;

static int mb_list_insert(mb_list_item_t **list, mb_list_item_t *item)
{
	/* check input parameters */
	if (!item) {
		while(1);
		return -1;
	}

	/* 1. no node in current list */
	if (!(*list)) {
		*list = item;
		return 0;
	}

	return mb_list_insert(&((*list)->next), item);
}

mb_list_item_t * mb_get_listItem(mb_list_item_t **list, uint16_t ItemStart , uint16_t ItemEnd)
{
	uint16_t list_start_addr;
	uint16_t list_end_addr;

	if (!(*list))
		return NULL;

	list_start_addr = (*list)->addr;
	list_end_addr = list_start_addr + (*list)->len;

	if (list_start_addr <= ItemStart && list_end_addr > ItemEnd)
		return *list;

	return mb_get_listItem(&((*list)->next), ItemStart, ItemEnd);
}


static int mb_reg_ComBlock_register(mb_com_pcb_t *pcb, int type, uint16_t addr, uint16_t len)
{
	mb_list_item_t *item;
	uint32_t hold_reg_len;
	if (!len)
		return -1;

	if (type == MB_TYPE_HOLD) {
		hold_reg_len = hold_data_struct_length();
		item = (mb_list_item_t *)mb_malloc(sizeof(*item) + hold_reg_len *len);
		if (!item) //malloc fail
			return -1;

		/* clear all content */
		memset((void *)item, 0, sizeof(*item) + sizeof(uint16_t) *len);

		item->addr = addr;
		item->len = len;
		item->data = (void *)((uint32_t)item + sizeof(*item));

		/* initial data zone */
		hold_data_zone_init(item->data, len);

		if (mb_list_insert(&(pcb->list.item), item))
			return -1;

		/* recode segment number */
		pcb->list.seg_num ++;
	}
	return 0;
}

int mb_slave_reg_ComBlock_register(int type, uint16_t addr, uint16_t len)
{
	return mb_reg_ComBlock_register(&slave_com_pcb, type, addr, len);
}

static mb_list_item_t *mb_get_item_by_segment(mb_com_pcb_t *pcb, int type, uint16_t start, uint16_t end)
{
	if (type == MB_TYPE_HOLD) {
		return mb_get_listItem(&pcb->list.item, start, end);
	}

	return NULL;
}


mb_list_item_t *mb_slave_get_item_by_segment(int type, uint16_t start, uint16_t end)
{
	return mb_get_item_by_segment(&slave_com_pcb, type, start, end);
}

static mb_list_item_t * mb_get_item_by_addr(mb_com_pcb_t *pcb, int type, uint16_t ser_val)
{
	if (type == MB_TYPE_HOLD) {
		return mb_get_listItem(&pcb->list.item, ser_val, ser_val);
	}

	return NULL;
}

mb_list_item_t * mb_slave_get_item_by_addr(int type, uint16_t val)
{
	return mb_get_item_by_addr(&slave_com_pcb, type, val);
}

static int mb_reg_segment_init(void)
{
	memset((void *)&slave_com_pcb.list, 0, sizeof(mb_list_t));
	slave_com_pcb.list.mb_type = MB_TYPE_HOLD;
	return 0;
}

unsigned char * mb_error(unsigned char error, unsigned char id, unsigned char func)
{
	unsigned short crc;

	((error_code_t *)error_buf)->device_id = id;
	((error_code_t *)error_buf)->function_id = func | 0x80;
	((error_code_t *)error_buf)->error_id = error;

	crc = usMBCRC16(error_buf, 3);

	((error_code_t *)error_buf)->crc = crc;

	return error_buf;
}

static int mb_main_process(void * mb_pcb, uint8_t *data, uint16_t len)
{
	mb_com_pcb_t * com_pcb;
	uint8_t func_code;
	uint16_t ret_len;
	uint16_t crc = 0;

	(void)len;
	if (mb_pcb == NULL)
		return -1;
	com_pcb = mb_pcb;

	/* check modbus block status */
	if (com_pcb->mb_status == MB_IDEL)
		return 0;

	/* function code is fix position */
	func_code = data[1];

	switch (func_code) {
		case 0x10:
		case 0x03:
		case 0x06:
			if (hold_process_main(com_pcb->hold, func_code, (uint16_t *)data, &ret_len))
				goto modbus_error;
			break;
		default:
				goto modbus_error;
			break;
	}

	if (ret_len) {
		/* set communication id */
		data[0] = com_pcb->id;
		/* calc CRC */
		crc = usMBCRC16(data, ret_len);
		*(uint16_t *)(data + ret_len) = crc;
		modbus_write(com_pcb->com, data, ret_len + 2);
	}

	return 0;

modbus_error:
	modbus_write(com_pcb->com, mb_error(illegal_function, data[0], func_code), ERROR_LEN);
	return -1;
}


static driving_msg_t driving_msg_prio;

/* run in interrupt mode */
/* period read slave registers */
void master_period_task(void *arg)
{
	(void)arg;

	taskENTER_CRITICAL();
	{
		if (!driving_msg_prio.used) {
			driving_msg_prio.type = BD_DRIVING_TYPE_HOLD_PERIO;
			/* increase period counter */
			driving_msg_prio.counter ++;

			driving_msg_prio.used = 1;
			/* send period event */
			driving_msg_send(&driving_msg_prio);
		}
	}
	taskEXIT_CRITICAL();
}

PRIVILEGED_DATA static TaskHandle_t modbus_driving_handle = NULL;

/* modbus dring upload */
static uint8_t driving_send_buf[128];
static void modbus_driving_upload(const void *arg)
{
	TimerHandle_t com_prio_handler;
	driving_msg_t *driv_msg;
	void * msg;
	static mb_com_pcb_t *com_pcb;
	uint16_t crc;
	uint16_t ret_len;

	if (arg == NULL)
		while(1);

	com_pcb = (mb_com_pcb_t *)arg;

	/* 100 ms refersh system status */
	com_prio_handler = xTimerCreate("mst_pri",
			1000,
			pdTRUE,
			0,
			com_pcb->period_task );

	if (!com_prio_handler)
		while(1);
	//xTimerStart(com_prio_handler, 0);
	/* create driving message */
	driving_mailbox = xQueueCreate(4, sizeof(void *));

	if (driving_mailbox == NULL) {
		sys_log_e(" create driving upload mailbox fail\n");
		while(1);
	}

	for (;;) {
		/* wait update event */
		xQueueReceive(driving_mailbox, &msg, portMAX_DELAY);

		driv_msg = (driving_msg_t *)msg;

		switch(DRIV_MSG_TYPE(driv_msg)) {
			/* period wtite */
			case BD_DRIVING_TYPE_HOLD_TIG:
			case BD_DRIVING_TYPE_HOLD_MSG:
				hold_driving_upload_proc(com_pcb->hold, driving_send_buf, &ret_len, driv_msg->reg_addr, driv_msg->reg_len);
			break;
			/* period read */
			case BD_DRIVING_TYPE_HOLD_PERIO:
				/* hold reg */
				hold_period_read_task(com_pcb->hold, driving_send_buf, &ret_len);
			break;
			/* trigger event */
		}

		taskENTER_CRITICAL();
		{
			/* clear used flag */
			driv_msg->used = 0;
		}
		taskEXIT_CRITICAL();

		if (ret_len) {
			driving_send_buf[0] = com_pcb->id;
			crc = usMBCRC16(driving_send_buf, ret_len);
			*(uint16_t *)(driving_send_buf + ret_len) = crc;
			modbus_write(com_pcb->com, driving_send_buf, ret_len + 2);
		}
	}
}

int driving_msg_send(void *msg)
{
	if (driving_mailbox == NULL)
		return -1;

	return xQueueSendToBack(driving_mailbox, &msg, 0);
}




void modbus_initial(int init_baud)
{
	master_com_pcb.mb_status = MB_IDEL;
	slave_com_pcb.mb_status = MB_IDEL;
	modbus_low_handler_init();
	/* initial modbus */
	modbus_phy_init(init_baud);

	mb_reg_segment_init();
}

int master_id_func_set(uint8_t id, uint8_t func_flag)
{
	void *com;

	master_com_pcb.id = id;
	com = modbus_device_id_binding(1, id, mb_main_process, (void *)&master_com_pcb);

	if (com == NULL)
		while(1);

	master_com_pcb.com = com;
	if (func_flag | MDOBUS_SUPPORT_HOLD) {
		master_com_pcb.hold = (void *)hold_reg_get_description_st(ROLE_MASTER);
	}

	master_com_pcb.mb_status = MB_INIT_DONE;
	/* inital period task */
	master_com_pcb.period_task = master_period_task;
	/* create driving thread */
	osThreadDef(modbus_driving_task, modbus_driving_upload, osPriorityNormal, 0, 128);
	modbus_driving_handle = osThreadCreate(osThread(modbus_driving_task), &master_com_pcb);

	return 0;
}

int slave_id_func_set(uint8_t id, uint8_t func_flag)
{
	void *com;

	slave_com_pcb.id = id;
	com = modbus_device_id_binding(0, id, mb_main_process, (void *)&slave_com_pcb);
	if (com == NULL)
		while(1);
	slave_com_pcb.com = com;
	if (func_flag | MDOBUS_SUPPORT_HOLD) {
		slave_com_pcb.hold = (void *)hold_reg_get_description_st(ROLE_SLAVE);
	}
	slave_com_pcb.mb_status = MB_INIT_DONE;
	return 0;
}

int master_data_send(uint8_t *data, uint16_t len)
{
	return modbus_write((const void *)master_com_pcb.com, data, len);
}

int slave_data_send(uint8_t *data, uint16_t len)
{
	return modbus_write((const void *)slave_com_pcb.com, data, len);
}

int master_holdReg_len(uint16_t len)
{
	return sci_set_hold_reg_len(master_com_pcb.hold, len);
}

int slave_holdReg_len(uint16_t len)
{
	return sci_set_hold_reg_len(slave_com_pcb.hold, len);
}

int mb_slave_description_sigle_addr(int type, uint16_t addr, uint16_t **data, void *ops, int perimission)
{
	mb_list_item_t * item;
	uint32_t data_array_size;
	uint32_t index;
	uint16_t * data_pt;
	/* check input address if legal */
	item = mb_slave_get_item_by_addr(type, addr);

	if (!item)
		return -1;

	if (!item->data)
		return -1;

	switch (type) {
		case MB_TYPE_CION:
			break;
		case MB_TYPE_HOLD:
			data_array_size = hold_data_struct_length();
			index = addr - item->addr;
			data_pt = hold_description_sigle_addr(
					item->data + index * data_array_size , ops,
					perimission);
			if (data)
				*data = data_pt;
			break;
		default :
			return -1;
			break;
	}

	return 0;
}

