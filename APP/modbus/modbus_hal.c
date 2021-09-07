#include "modubs_phy.h"
#include "modbus_hal.h"
#include "modbus_lib.h"
#include "mb.h"

/* static buf */
#define BUF_MAX_LEN	128
//#define MODBUS_LOOP
#define DEVICE_ID_INDEX		1
#define DEVICE_FUNC_INDEX	2

static uint8_t modbus_local_buf[BUF_MAX_LEN];
/* define driving structure */
struct modbus_driving_msg_t {
	int if_num;
	uint16_t up_addr;
	uint16_t up_len;
	int initial_flag;
};

static struct modbus_driving_msg_t local_driving_msg = {
	.initial_flag = 0,	
};


void * modbus_device_id_binding(int if_num, uint16_t id, 
		int (*id_func)(void *, uint8_t *, uint16_t),
	void *parm)
{
	void *com;

	modbus_id_binding(if_num, id, id_func, parm, &com);

	return com;
}

static void local_data_processing(const void *arg)
{
	modbus_phy_t * msg;
	int recv_data_len;
	uint8_t device_id = 0;
	int i; 
	(void) arg;

	while(1) {
		/* wait forever */
		xQueueReceive(modbus_maxbox, &msg, portMAX_DELAY);

		if (msg == NULL) {
			continue;
		}

		recv_data_len = msg->ops->read((void *)msg, modbus_local_buf, BUF_MAX_LEN);

#if defined (MODBUS_LOOP)
		msg->ops->write((void *)msg, modbus_local_buf, recv_data_len);
		continue;
#endif

		if (recv_data_len == -1) {
			sys_log_e("read %s buf fail",msg->name);
			continue;
		}
		/* calc CRC and data len and check ID*/
		if ((usMBCRC16(modbus_local_buf, recv_data_len) != 0) || 
					recv_data_len < MB_SER_PDU_SIZE_MIN) {
			/* ignore */
			continue;
		}

		/* check board id */
		device_id = modbus_local_buf[DEVICE_ID_INDEX -1];
		for (i = 0; i < msg->current_device_id_num; i++)
		{
			if (msg->device_id->id != device_id)
				continue;
			/* find match id */
			if (msg->device_id->id_func != NULL) {
				msg->device_id->id_func(msg->device_id->param, modbus_local_buf, recv_data_len);
				goto done;
			}
		}
		
		msg->ops->write((void *)msg,\
				mb_error(illegal_function, device_id, modbus_local_buf[DEVICE_FUNC_INDEX -1]),\
				ERROR_LEN);
done:
		;
	}
}

PRIVILEGED_DATA static TaskHandle_t modbus_low_handle = NULL;

void modbus_low_handler_init(void)
{
  osThreadDef(modbus_low_task, local_data_processing, osPriorityNormal, 0, 512);
  modbus_low_handle = osThreadCreate(osThread(modbus_low_task), NULL);
}

