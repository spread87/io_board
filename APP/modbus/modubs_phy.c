#include "timer_bsp.h"
#include "uart_bsp.h"
#include "modubs_phy.h"
#include "RS485_bsp.h"

static modbus_phy_t sys_modbus_phy[] = {
	{
#define _RS485_IF_1_
		.name = "RS485_IF_1",
	},
};

void modbus_recv_timer(void *arg);
void modbus_recv_data_1_byte(void *arg, uint8_t data);
int modbus_write(const void *thiz, uint8_t * data, uint16_t len);
int modbus_read(const void *thiz, uint8_t * data, uint16_t len);

modbus_ops_t modbus_phy_ops = {
	.write = modbus_write,
	.read = modbus_read,
};

void modbus_phy_init(int init_baud)
{
	int i;
	timer_ops_t * timer_ops;
	struct RS485_ops * rs485_ops;
	uint32_t baud;
	uint32_t timer_cnt = 0;

	sys_modbus_phy[0].com_if = module_get(0, "RS485IF1");
	if (sys_modbus_phy[0].com_if == NULL) {
		sys_log_e("get RS485 1 module fail\n");
		while(1);
	}

	sys_modbus_phy[0].timer_if = module_get(0, "tim2");
	if (sys_modbus_phy[0].timer_if == NULL) {
		sys_log_e("get TIMER 6 module fail\n");
		while(1);
	}
#ifdef _RS485_IF_2_
	sys_modbus_phy[1].com_if = module_get(0, "RS485IF2");
	if (sys_modbus_phy[0].com_if == NULL) {
		sys_log_e("get RS485 2 module fail\n");
		while(1);
	}

	sys_modbus_phy[1].timer_if = module_get(0, "tim3");
	if (sys_modbus_phy[1].timer_if == NULL) {
		sys_log_e("get TIMER 7 module fail\n");
		while(1);
	}
#endif
	for (i = 0; i < (int)(sizeof(sys_modbus_phy) / sizeof(modbus_phy_t)); i++) {
		sys_modbus_phy[i].ops = &modbus_phy_ops;
		timer_ops = sys_modbus_phy[i].timer_if->ops;
		if (timer_ops == NULL) {
			while(1);
		}

		rs485_ops = sys_modbus_phy[i].com_if->ops;
		if (rs485_ops == NULL) {
			while(1);
		}

		/* phy initial */
		timer_ops->init(sys_modbus_phy[i].timer_if);
		rs485_ops->init(sys_modbus_phy[i].com_if);

		/* initial callback function */
		rs485_ops->register_call(sys_modbus_phy[i].com_if,
				modbus_recv_data_1_byte, (void *)&sys_modbus_phy[i]);
		rs485_ops->set_baud(sys_modbus_phy[i].com_if, init_baud);
		/* set timer accroding to baud */
		baud = rs485_ops->get_baud(sys_modbus_phy[i].com_if);
		timer_cnt = (uint32_t)((1000000.0 / baud) * 10 * 4);

		sys_modbus_phy[i].timer_val = timer_cnt;
		timer_ops->create_timer(sys_modbus_phy[i].timer_if, timer_cnt,
				modbus_recv_timer, (void *)&sys_modbus_phy[i], 0);

		if (i < MODBUS_IF_MAX) {
			sys_modbus_phy[i].mb_if_num = i;
		}

		/* clear all app register */
		sys_modbus_phy[i].current_device_id_num = 0;
		sys_modbus_phy[i].device_id->id = 0;
		sys_modbus_phy[i].device_id->id_func = NULL;
		sys_modbus_phy[i].device_id->param = NULL;
	}

	/* create queue */
	modbus_maxbox = xQueueCreate(MODBUS_MAILBOX_LEN, sizeof(void *));
	if (modbus_maxbox == NULL) {
		sys_log_e("modbus periphys create mailbox fail\n");
		while(1);
	}
}

/* modubs phy layout */
void modbus_recv_data_1_byte(void *arg, uint8_t data)
{
	modbus_phy_t *modbus_phy;
	timer_ops_t * timer_ops;

	if (arg == NULL) {
		return ;
	}

	modbus_phy = (modbus_phy_t *)arg;

	timer_ops = (timer_ops_t *)modbus_phy->timer_if->ops;

	timer_ops->reload(modbus_phy->timer_if, modbus_phy->timer_val, 1);

	/* wait analyse */
	if (modbus_phy->data.flag)
		return;

	/* check current buffer length */
	if (modbus_phy->data.cur_ptr >= MODBUS_RECV_BUF_LEN)
		return;

	modbus_phy->data.data[modbus_phy->data.cur_ptr++] = data;
}

void modbus_recv_timer(void *arg)
{
	modbus_phy_t *modbus_phy;
	timer_ops_t * timer_ops;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint32_t msg;

	if (arg == NULL) {
		return ;
	}

	modbus_phy = (modbus_phy_t *)arg;
	modbus_phy->data.flag = 1;
	timer_ops = modbus_phy->timer_if->ops;
	timer_ops->stop(modbus_phy->timer_if);

	if (xQueueIsQueueFullFromISR(modbus_maxbox)) {
		return ;
	}
	msg = (uint32_t)arg;
	xQueueSendFromISR(modbus_maxbox, &msg,&xHigherPriorityTaskWoken);

	/* switch task if find higher task */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int modbus_write(const void *thiz, uint8_t * data, uint16_t len)
{
	modbus_phy_t *modbus_phy;
	struct RS485_ops * rs485_ops;

	if (thiz == NULL) {
		return -1;
	}

	modbus_phy = (modbus_phy_t *)thiz;
	rs485_ops = (struct RS485_ops *)modbus_phy->com_if->ops;

	rs485_ops->write(modbus_phy->com_if, data, len);

	return len;
}

int modbus_read(const void *thiz, uint8_t * data, uint16_t len)
{
	modbus_phy_t * modbus_phy;
	int i, buf_len;

	if (thiz == NULL) {
		return -1;
	}

	modbus_phy = (modbus_phy_t *)thiz;

	buf_len = modbus_phy->data.cur_ptr;

	if (len < buf_len) {
		return -1;
	}

	for (i = 0; i < buf_len; i++) {
		data[i] = modbus_phy->data.data[i];
	}

	/* restore buf status */
	taskENTER_CRITICAL();

	modbus_phy->data.flag = 0;
	modbus_phy->data.cur_ptr = 0;

	taskEXIT_CRITICAL();

	return buf_len;
}

int modbus_id_binding(int if_num, uint8_t id,
	int (*id_func)(void *, uint8_t *, uint16_t),
	void *para, void **com)
{

	if (if_num >= MODBUS_IF_MAX) {
		return -1;
	}

	if (id_func == NULL)
		return -1;

	if (sys_modbus_phy[if_num].current_device_id_num < MODBUS_MAX_ID_NUM) {
		sys_modbus_phy[if_num].device_id->id = id;
		sys_modbus_phy[if_num].device_id->id_func = id_func;
		sys_modbus_phy[if_num].device_id->param = para;
		sys_modbus_phy[if_num].current_device_id_num++;
		*com = (void *)&sys_modbus_phy[if_num];
	} else {
		return -1;
	}

	return 0;
}

