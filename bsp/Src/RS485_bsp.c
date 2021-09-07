#include "RS485_bsp.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"

#include "usart.h"
#include "gpio_bsp.h"
#include "uart_bsp.h"


typedef struct {
	module_t rs485_mod;
	rs485_mod_private_t rs485_private;
}rs485_set_t;

rs485_set_t rs485_set[] = {
	{
		.rs485_mod.name = "RS485IF1",
		.rs485_private.gpio_inst.gpio_em = P_RS485_RW_CTRL,
		.rs485_private.uart_inst.name = "UART1",
	},
};

static SemaphoreHandle_t rs485_lock = NULL;
static SemaphoreHandle_t rs485_async_semap = NULL;

static void rs485_transmit_done_callback(void *arg);

void RS485_Init(void *thiz)
{
	module_t * RS485_GPIO_inst;
	module_t * RS485_UART_inst;
	struct GPIO_ops *gpio_ops;
	uart_ops_t *uart_ops;
	int deafult_485_rw_pin_val = 0;
	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;

	if (rs485_inst == NULL) {
		sys_log_e("invalid rs485\n");
		while(1);
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (rs485_pri == NULL) {
		sys_log_e("invalid private data\n");
		while(1);
	}

	RS485_GPIO_inst = module_get(0, "gpio");

	if (RS485_GPIO_inst == NULL) {
		sys_log_e("get gpio module fail!!!\n");
		while(1);
	}
	rs485_pri->gpio_inst.gpio_inst = RS485_GPIO_inst;

	RS485_UART_inst = module_get(0, rs485_pri->uart_inst.name);
	if (RS485_UART_inst == NULL) {
		sys_log_e("get uart module fail!!!\n");
		while(1);
	}

	rs485_pri->uart_inst.uart_inst = RS485_UART_inst;

	gpio_ops = RS485_GPIO_inst->ops;
	if (gpio_ops == NULL) {
		sys_log_e("get gpio operation function fail\n");
		while(1);
	}

	uart_ops = RS485_UART_inst->ops;
	if (uart_ops == NULL) {
		sys_log_e("get uart operation function fail\n");
		while(1);
	}

	/* initial periphys */
	gpio_ops->init((void *)RS485_GPIO_inst);
	uart_ops->init((void *)RS485_UART_inst);
	uart_ops->register_txdone_call((void*)RS485_UART_inst,
			rs485_transmit_done_callback, thiz);

	/* set 485 dirction is input */
	gpio_ops->write(&RS485_GPIO_inst, rs485_pri->gpio_inst.gpio_em,
			(void *)&deafult_485_rw_pin_val);

	rs485_lock = xSemaphoreCreateMutex();
	if (rs485_lock == NULL) {
		sys_log_e("get lock fail\n");
		while(1);
	}

	rs485_async_semap = xSemaphoreCreateBinary();
	if (rs485_async_semap == NULL) {
		sys_log_e("get lock fail\n");
		while(1);
	}


}
static void rs485_transmit_done_callback(void *arg)
{
	static portBASE_TYPE xHigherPriorityTaskWoken;
	(void) arg;

	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR( rs485_async_semap, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void RS485_raw_write(void *thiz,
		uint8_t *data,
		uint16_t len)
{
	int rs485_rw_pin_val = 0;
	struct GPIO_ops *gpio_ops;
	uart_ops_t *uart_ops;
	uint32_t gpio_em;
	uart_private_t *uart_pri;

	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;

	if (rs485_inst == NULL) {
		sys_log_e("invalid rs485\n");
		while(1);
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (rs485_pri == NULL) {
		sys_log_e("invalid private data\n");
		while(1);
	}

	gpio_ops = rs485_pri->gpio_inst.gpio_inst->ops;
	if (gpio_ops == NULL) {
		sys_log_e("get gpio operation function fail\n");
		while(1);
	}

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;
	if (uart_ops == NULL) {
		sys_log_e("get uart operation function fail\n");
		while(1);
	}

	gpio_em = rs485_pri->gpio_inst.gpio_em;

	/* get lock */
	xSemaphoreTake(rs485_lock, osWaitForever);

	rs485_rw_pin_val = 1;
	/* set 485 dirctory */
	gpio_ops->write(rs485_pri->gpio_inst.gpio_inst, gpio_em,
			(void *)&rs485_rw_pin_val);

	//uart_raw_send(data, len);
	uart_ops->write(rs485_pri->uart_inst.uart_inst, data, len);

	/* wait transmit done */
	xSemaphoreTake(rs485_async_semap, portMAX_DELAY);

	uart_pri = rs485_pri->uart_inst.uart_inst->private;

	while ((!LL_USART_IsActiveFlag_TXE(uart_pri->base)) ||
			!LL_USART_IsActiveFlag_TC(uart_pri->base));
	/* set 485 dirctory */
	rs485_rw_pin_val = 0;
	gpio_ops->write(rs485_pri->gpio_inst.gpio_inst, gpio_em,
			(void *)&rs485_rw_pin_val);

	/* release lock */
	xSemaphoreGive(rs485_lock);
}

int RS485_recv_call_back_register(void *thiz,
		void (*call) (void *, uint8_t),
		void *arg)
{
	uart_ops_t *uart_ops;
	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;

	if (rs485_inst == NULL) {
		sys_log_e("invalid rs485\n");
		while(1);
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (rs485_pri == NULL) {
		sys_log_e("invalid private data\n");
		while(1);
	}

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;
	if (uart_ops == NULL) {
		sys_log_e("get uart operation function fail\n");
		while(1);
	}

	uart_ops->register_rx_call(rs485_pri->uart_inst.uart_inst, call, arg);

	return 0;
}

static uint32_t rs485_get_baud(void *thiz)
{
	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;
	uart_ops_t * uart_ops;

	if (rs485_inst == NULL) {
		return 0;
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (rs485_pri == NULL) {
		return 0;
	}

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;

	return uart_ops->get_baud(rs485_pri->uart_inst.uart_inst);
}

static int rs485_set_baud(void *thiz, uint32_t baud)
{
	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;
	uart_ops_t * uart_ops;

	if (rs485_inst == NULL) {
		return 0;
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (rs485_pri == NULL) {
		return 0;
	}

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;

	return uart_ops->set_baud(rs485_pri->uart_inst.uart_inst, baud);
}

static int rs485_set_parity(void *thiz, uint32_t parity)
{
	module_t * rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;
	uart_ops_t *uart_ops;


	if (!rs485_inst) {
		return -1;
	}

	rs485_pri = (rs485_mod_private_t *)rs485_inst->private;

	if (!rs485_pri)
		return -1;

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;

	return uart_ops->set_parity(rs485_pri->uart_inst.uart_inst, parity);
}

static uint32_t rs485_get_parity(void *thiz)
{
	module_t *rs485_inst = (module_t *)thiz;
	rs485_mod_private_t *rs485_pri;
	uart_ops_t *uart_ops;

	if (!rs485_inst) {
		return -1;
	}

	rs485_pri = rs485_inst->private;
	if (!rs485_pri) {
		return -1;
	}

	uart_ops = rs485_pri->uart_inst.uart_inst->ops;

	return uart_ops->get_parity(rs485_pri->uart_inst.uart_inst);
}

struct RS485_ops rs485_operation_func = {
	.init = RS485_Init,
	.write = RS485_raw_write,
	.register_call = RS485_recv_call_back_register,
	.get_baud = rs485_get_baud,
	.set_baud = rs485_set_baud,
	.set_parity = rs485_set_parity,
	.get_parity = rs485_get_parity,
};

static int rs485_module_register(void)
{
	int i;

	for (i = 0; i < (int)(sizeof(rs485_set) / sizeof(rs485_set_t)); i++) {
		rs485_set[i].rs485_mod.ops = (void *)&rs485_operation_func;
		rs485_set[i].rs485_mod.private = (void *)&rs485_set[i].rs485_private;
		module_register(RS485_IF_MODULE + i, &rs485_set[i].rs485_mod);
	}
	return 0;
}

device_initcall(rs485_module_register);

