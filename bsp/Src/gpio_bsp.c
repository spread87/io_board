#include "gpio_bsp.h"
#include "module.h"
#include "sci.h"
#include "main.h"
#include "gpio.h"
#include "stm32f103xb.h"
#include "stm32f1xx_ll_gpio.h"

#define PIN_AUTO_CTRL(con, port, pin) do{\
	if (con) \
		HAL_GPIO_WritePin(port, (uint16_t)pin, GPIO_PIN_SET); \
	else \
		HAL_GPIO_WritePin(port, (uint16_t)pin, GPIO_PIN_RESET); \
}while(0)

static module_t gpio_module = {
	.name = "gpio",
};

static void gpio_init(void *thiz)
{
	static int init_flag = 0;
	(void) thiz;

	if (!init_flag) {
		MX_GPIO_Init();
		init_flag ++ ;
	}
}

static void gpio_write(void *thiz,
		enum GPIO_FUNC_KEY key,
		void *value)
{
	if (value == NULL) {
		sys_log_e("Invalid parameter!!\r\n");
		while(1);
	}

	(void)thiz;
	switch (key) {
		case P_RS485_RW_CTRL:
			PIN_AUTO_CTRL(*(int *)value, RS485_CTL_GPIO_Port, RS485_CTL_Pin);
			break;
		case P_LED_1_CTRL:
			PIN_AUTO_CTRL(*(int *)value, SYS_LED0_GPIO_Port, SYS_LED0_Pin);
			break;
		case P_LED_2_CTRL:
			PIN_AUTO_CTRL(*(int *)value, SYS_LED1_GPIO_Port, SYS_LED1_Pin);
			break;
		case P_DO0:
			PIN_AUTO_CTRL(*(int *)value, DO1_GPIO_Port, DO1_Pin);
			break;
		case P_DO1:
			PIN_AUTO_CTRL(*(int *)value, DO2_GPIO_Port, DO2_Pin);
			break;
		case P_DO2:
			PIN_AUTO_CTRL(*(int *)value, DO3_GPIO_Port, DO3_Pin);
			break;
		case P_DO3:
			PIN_AUTO_CTRL(*(int *)value, DO4_GPIO_Port, DO4_Pin);
			break;
		case P_EEPROM_SCL:
			PIN_AUTO_CTRL(*(int *)value, EEPROM_SCL_GPIO_Port, EEPROM_SCL_Pin);
			break;
		case P_EEPROM_SDA:
			PIN_AUTO_CTRL(*(int *)value, EEPROM_SDA_GPIO_Port, EEPROM_SDA_Pin);
			break;
		default:
			;
		break;
	}
}

static int gpio_read(void *thiz, enum GPIO_FUNC_KEY key)
{
	GPIO_PinState gpio_status;
	if (thiz == NULL) {
		sys_log_e("Invalid parameter\r\n");
		while(1);
	}

	switch (key) {
		case P_EEPROM_SDA:
			gpio_status = HAL_GPIO_ReadPin(EEPROM_SDA_GPIO_Port, (uint16_t)EEPROM_SDA_Pin);
			break;
		case P_DI0:
			gpio_status = HAL_GPIO_ReadPin(DI1_GPIO_Port, (uint16_t)DI1_Pin);
			break;
		case P_DI1:
			gpio_status = HAL_GPIO_ReadPin(DI2_GPIO_Port,(uint16_t) DI2_Pin);
			break;
		case P_DI2:
			gpio_status = HAL_GPIO_ReadPin(DI3_GPIO_Port,(uint16_t) DI3_Pin);
			break;
		case P_DI3:
			gpio_status = HAL_GPIO_ReadPin(DI4_GPIO_Port,(uint16_t) DI4_Pin);
			break;
		case P_DO0:
			gpio_status = HAL_GPIO_ReadPin(DO1_GPIO_Port, (uint16_t)DO1_Pin);
			break;
		case P_DO1:
			gpio_status = HAL_GPIO_ReadPin(DO2_GPIO_Port,(uint16_t) DO2_Pin);
			break;
		case P_DO2:
			gpio_status = HAL_GPIO_ReadPin(DO3_GPIO_Port,(uint16_t) DO3_Pin);
			break;
		case P_DO3:
			gpio_status = HAL_GPIO_ReadPin(DO4_GPIO_Port,(uint16_t) DO4_Pin);
			break;

		default:
			return -1;
		break;
	}

	return gpio_status == GPIO_PIN_SET;
}

static int gpio_mode_cfg(void *thiz, 
				enum GPIO_FUNC_KEY key,
				uint32_t mode)
{
	(void)thiz;
	(void)mode;

	switch (key) {
		default:
			return -1;
		break;
	}
}

static struct GPIO_ops gpio_operation_ops = {
	.init = gpio_init,
	.write = gpio_write,
	.read = gpio_read,
	.mode = gpio_mode_cfg,
};

static int gpio_module_register(void)
{
	gpio_module.ops = (void *)&gpio_operation_ops;
	module_register(GPIO_MODULE, &gpio_module);
	return 0;
}

device_initcall(gpio_module_register);

