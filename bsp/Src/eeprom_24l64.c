#include "module.h"
#include "sci.h"
#include "eeprom_26l64.h"
#include "i2c_bsp.h"
#include "gpio_bsp.h"
#include "cmsis_os.h"

module_t eeprom_mod = {
	.name = "eeprom",
};

static module_t *e_gpio_inst;
static module_t *e_i2c_inst;
static i2c_private_t i2c_prv;

static void eeprom_init(void *thiz)
{
	i2c_ops_t *i2c_ops;
	int n =0;
	struct GPIO_ops *gpio_ops;
	if (!thiz)
		while(1);

	e_gpio_inst = module_get(0, "gpio");

	if (!e_gpio_inst)
		while(1);

	
	gpio_ops = e_gpio_inst->ops;

	gpio_ops->init(e_gpio_inst);

	i2c_prv.gpio_mod = e_gpio_inst;
	i2c_prv.i2c_scl_pin = P_EEPROM_SCL;
	i2c_prv.i2c_sda_pin = P_EEPROM_SDA;
	i2c_prv.device_id = EEPROM_DEVICE_ID;

	e_i2c_inst = module_get(0, "i2c");

	if (!e_i2c_inst)
		while(1);

	i2c_ops = e_i2c_inst->ops;

	i2c_ops->init(e_i2c_inst, &i2c_prv);

	while (!Isok_i2c_Device())
		if (n++ > 500)
			udelay(2000);
}

static int eeprom_write(void *thiz, uint16_t addr, uint8_t data)
{
	uint8_t tx_buf[3];
	int ret;
	uint8_t temp = 0;

	if (!thiz)
		return -1;

	tx_buf[0] = addr >> 8;
	tx_buf[1] = addr & 0xFF;
	tx_buf[2] = data;

	/* setp 1. read current addr value */

	taskENTER_CRITICAL();
	{
		IIC_Read_Buffer(2, tx_buf, 1, &temp);
		if (temp == data)
			ret = 1;
		else
			ret = IIC_Write_Buffer(3, tx_buf);
		IIC_Read_Buffer(2, tx_buf, 1, &temp);
		if (temp == data)
			ret = 1;
		else
			ret = 0;
	}
	taskEXIT_CRITICAL();

	return !ret;
}

static int eeprom_read(void *thiz, uint16_t addr, uint16_t len, uint8_t *rx_buf)
{
	uint8_t tx_buf[2];
	int ret = 0;
	(void) thiz;

	tx_buf[0] = addr >> 8;
	tx_buf[1] = addr & 0xFF;


	taskENTER_CRITICAL();
	{
		ret = IIC_Read_Buffer(2, tx_buf, len, rx_buf);
	}
	taskEXIT_CRITICAL();
	return !ret;
}

eeprom_ops_t e_ops = {
	.init = eeprom_init,
	.write = eeprom_write,
	.read = eeprom_read,
};

static int eeprom_module_register(void)
{
	eeprom_mod.ops = &e_ops;

	module_register(EEPROM_MODULE, &eeprom_mod);
	return 0;
}

device_initcall(eeprom_module_register);
