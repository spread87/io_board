#include "mb.h"
#include "./reg.h"
#include "string.h"

#define HOLD_REG_MAGIC		0xFF
#define _hold_permission_set(x)	(0xF0|(x & 0xf))

#define HOLD_PERMISSION_CONVERSION(x) \
			_hold_permission_set(x)
#define HOLD_PERMISSION_RO \
			_hold_permission_set(MB_PERMISSION_RO)
#define HOLD_PERMISSION_VOLATILE \
			_hold_permission_set(MB_PERMISSION_VOLATILE)

typedef struct {
	uint16_t reg[HOLD_REGISTERS_NUMBER];
	reg_func reg_call[HOLD_REGISTERS_NUMBER];
}hold_reg_desreg_t;

/* hold type data */
typedef struct {
	union {
		struct {
			uint8_t mask;
			uint8_t permission;
			uint16_t reg;
		};
		uint16_t (*ops) (int type, uint16_t val);
	};
}reg_data_t;

/* default register permission is read only */
reg_data_t default_data = (reg_data_t) {
	.mask = HOLD_REG_MAGIC,
	.permission = HOLD_PERMISSION_RO,
	.reg = 0x0000,
};
/* define hold reg struct */

/* define local hold registers */
static hold_reg_t master_hold_reg;
static hold_reg_t slave_hold_reg;

int hold_data_struct_length(void)
{
	return sizeof(reg_data_t);
}

int hold_data_zone_init(void * data_p, uint16_t len)
{
	int i;
	reg_data_t *data;

	if (!data_p || !len) {
		while(1);
		return -1;
	}

	data = data_p;

	for (i = 0; i < len; i++) {
		memcpy(&data[i], &default_data, sizeof(default_data));
	}

	return 0;
}

int hold_data_zone_register(uint16_t addr, 
		uint16_t (*ops) (int, uint16_t))
{
	reg_data_t *reg_data;
	mb_list_item_t * item;

	item = mb_slave_get_item_by_addr(MB_TYPE_HOLD, addr);
	if (!item)
		return -1;

	if (!ops)
		return -1;

	reg_data = item->data;
	
	reg_data[addr-item->addr].ops = ops;

	return 0;
}

uint16_t * hold_description_sigle_addr(void *regPool, void *in_data, int perimission)
{
	reg_data_t *reg_data = regPool;

	switch(HOLD_PERMISSION_CONVERSION(perimission)) {
		case HOLD_PERMISSION_RO :
			reg_data->permission = HOLD_PERMISSION_RO;
			reg_data->mask = HOLD_REG_MAGIC;
			return &(reg_data->reg);
			break;
		case HOLD_PERMISSION_VOLATILE :
			reg_data->ops = (uint16_t (*) (int, uint16_t)) (in_data);
			break;
	};
	return NULL;
}


int read_hold_reg(hold_reg_t * hold_st, uint16_t *buf, void *reg_array, uint16_t len)
{
	int i;
	uint16_t *recv_buf = buf;
	reg_data_t *reg_data;

	if (!hold_st || !reg_array) {
		return -1;
	}

	reg_data = reg_array;

	for (i = 0; i < len; i++) {
		if (reg_data[i].mask == HOLD_REG_MAGIC) 
		{
			if (reg_data[i].permission == HOLD_PERMISSION_RO)
				recv_buf[i] = htons(reg_data[i].reg);
		}
		else {
			if (reg_data[i].ops)
				recv_buf[i] = htons(reg_data[i].ops(MB_HOLD_READ, 0));
		}
	}

	return 0;
}

static int write_hold_reg(hold_reg_t * hold_st, uint16_t *data, void *reg_array ,uint16_t len)
{
	int i;
	reg_data_t *reg_data;

	if (!hold_st || !reg_array) {
		return -1;
	}

	reg_data = reg_array;

	for (i = 0; i < len; i++) {
		if (reg_data[i].mask == HOLD_REG_MAGIC) 
		{
			if (reg_data[i].permission == HOLD_PERMISSION_RO)
				continue;
			reg_data[i].reg = htons(data[i]);
		}
		else {
			if (reg_data[i].ops)
				reg_data[i].ops(MB_HOLD_WRITE, htons(data[i]));
		}
	}

	return 0;
}

/* register call back function */
static driving_msg_t driving_msg_msg;
static driving_msg_t driving_msg_trig;
/* slave hold register process */
static int slave_hold_process_main(void *hold_st, uint8_t func_code, uint16_t *data, uint16_t *len)
{
	slave_hold_reg_head_t *reg_head;
	slave_hold_reg_pdu_t * reg_10_st;
	slave_hold_read_t * reg_read_st;
	slave_hold_06_reg_t * reg_06_st;
	mb_list_item_t * item;
	reg_head = (slave_hold_reg_head_t*) data;
	uint16_t msg_len = 0;
	uint16_t reg_addr, reg_len, ms_addr;
	uint32_t index;
	reg_data_t *data_array;

	/* defaule length */
	if (hold_st == NULL)
		return 0;

	*len = 0;
	//slave master hold register mapping address 
	ms_addr = ((hold_reg_t *)hold_st)->ms_map_addr;
	switch(func_code) {
		case 0x06:
			reg_06_st = (slave_hold_06_reg_t *) data;

			reg_addr = htons(reg_06_st->addr);
			reg_len = 1;

			/* check addres if exist */
			item = mb_slave_get_item_by_segment(MB_TYPE_HOLD, reg_addr, reg_addr + reg_len - 1);
			if (!item)
				return 0;

			index = reg_addr - item->addr;
			data_array = item->data;

			if (write_hold_reg(hold_st, reg_06_st->data, &(data_array[index]), reg_len))
				return 0;
			*len = sizeof(slave_hold_reg_head_t);
			break;
		case 0x10: /* write event */
			reg_10_st = (slave_hold_reg_pdu_t *) data;
			reg_addr = htons(reg_head->addr);
			reg_len = htons(reg_head->reg_num);
			
			/* check addres if exist */
			item = mb_slave_get_item_by_segment(MB_TYPE_HOLD, reg_addr, reg_addr + reg_len - 1);
			if (!item)
				return 0;

			index = reg_addr - item->addr;
			data_array = item->data;
			if (write_hold_reg(hold_st, reg_10_st->data, &(data_array[index]), reg_len))
				return 0;
			*len = sizeof(slave_hold_reg_head_t);

			break;
		case 0x03: /* read event */
			msg_len = 3 + (htons(reg_head->reg_num) << 1);
			reg_read_st = (slave_hold_read_t *)data;
			reg_addr = htons(reg_head->addr);
			reg_len = htons(reg_head->reg_num);
			reg_read_st->ret_len = reg_len << 1;
			/* check addres if exist */
			item = mb_slave_get_item_by_segment(MB_TYPE_HOLD, reg_addr, reg_addr + reg_len - 1);
			if (!item)
				return 0;

			index = reg_addr - item->addr;
			data_array = item->data;

			if (read_hold_reg(hold_st, reg_read_st->data, &(data_array[index]), reg_len))
				return 0;
			*len = msg_len;
			break;
	}

	return 0;
}

int hold_process_main(void *hold_st, uint8_t func_code, uint16_t *data, uint16_t *len)
{
	hold_reg_t *hold_pcb_st;

	if (hold_st == NULL)
		return -1;
	hold_pcb_st = (hold_reg_t *)hold_st;

	hold_pcb_st->process(hold_st, func_code, data, len);

	return 0;
}
/* master hold register process */
struct master_reg_record_t {
	uint16_t addr;
	uint16_t len;
};

/* record laster data */ 
static struct master_reg_record_t local_rod_write = {0, 0};
static struct master_reg_record_t local_rcd_read = {0, 0};

#if defined (SUPPORT_HOLD_REG_MASTER)
static int master_hold_process_main(void *hold_st, uint8_t func_code, uint16_t* data, uint16_t *len)
{
	*len = 0;
	master_hold_read_t * reg_read_st;

	switch(func_code) {
		case 0x10:
			/*pass*/
			break;
		case 0x03:
			reg_read_st = (master_hold_read_t *)data;
			if (local_rod_write.len != (reg_read_st->ret_len >> 1))
				return -1;
			if (write_hold_reg(hold_st, reg_read_st->data, local_rod_write.addr, local_rod_write.len, REG_SYNC))
				return -1;
			break;
	}
	return 0;
}

int hold_driving_upload_proc(void *hold_st, uint8_t *data, uint16_t *ret_len, uint16_t addr, uint16_t addr_len)
{
	(void)hold_st;
	(void)data;
	(void)ret_len;
	(void)addr;
	(void)addr_len;
	return 0;
}
#endif

int hold_period_read_task(void *hold_st, uint8_t *data, uint16_t *len)
{
	uint16_t master_len;
	slave_hold_reg_head_t * hold_03_st;

	if (hold_st == NULL)
		return -1;
	if (data == NULL)
		return -1;

	hold_03_st = (slave_hold_reg_head_t*)data;
	master_len = master_hold_reg.max_len;
	if (!master_len)
		return -1;

	hold_03_st->func_num = 0x03;
	hold_03_st->addr = 0;
	hold_03_st->reg_num = htons(master_len);

	*len = sizeof(slave_hold_reg_head_t);
	return 0;
}

void * hold_reg_get_description_st(uint8_t role)
{
#if defined SUPPORT_HOLD_REG_MASTER
	if (role == ROLE_MASTER) {
		master_hold_reg.max_len = HOLD_REGISTERS_NUMBER;
		master_hold_reg.process = master_hold_process_main;
		return &master_hold_reg;
	}
#endif
#if defined SUPPORT_HOLD_REG_SLAVE
	if(role == ROLE_SLAVE) {
		slave_hold_reg.max_len = HOLD_REGISTERS_NUMBER;
		slave_hold_reg.process = slave_hold_process_main;
		slave_hold_reg.period_task = NULL;
		return &slave_hold_reg;
	}
#endif
	return NULL;
}

int sci_set_hold_reg_len(void *hold_st, uint16_t len)
{
	hold_reg_t * reg_role;

	if (hold_st == NULL) {
		while(1);
	}

	if (len == 0)
		len = 1;
	if (len > HOLD_REGISTERS_NUMBER)
		len = HOLD_REG_NUMBER;

	reg_role = (hold_reg_t *)hold_st;

	reg_role->max_len = len;

	return 0;
}

int sci_holdReg_MS_mapping(uint16_t sla_addr)
{
	slave_hold_reg.ms_map_addr = sla_addr;
	master_hold_reg.ms_map_addr = 0;
	return 0;
}

int modbus_driving_tigger_event(uint16_t addr, uint16_t len)
{

	taskENTER_CRITICAL();
	{
		driving_msg_trig.reg_addr = addr;
		driving_msg_trig.reg_len = len;
		driving_msg_trig.used = 1;
		driving_msg_trig.type = BD_DRIVING_TYPE_HOLD_TIG;
	}
	taskEXIT_CRITICAL();

	driving_msg_send(&driving_msg_trig);
	return 0;
}

