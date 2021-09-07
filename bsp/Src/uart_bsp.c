#include "uart_bsp.h"
#include "sci.h"
#include "module.h"
#include "usart.h"

typedef struct {
	module_t uart_mod;
	uart_private_t uart_private;
}uart_set_t;

uart_set_t uart_set[] = {
	{
		.uart_mod.name = "UART1",
#define UART_1_MOD_BASE		USART1
		.uart_private.base = UART_1_MOD_BASE,
	},
/*	{
		.uart_mod.name = "UART2",
#define UART_2_MOD_BASE		USART2
		.uart_private.base = UART_2_MOD_BASE,
	},
*/
};

static void uart_if_init(void *thiz)
{
	module_t * uart_mod = (module_t *)thiz;
	uart_private_t * uart_pri;

	if (uart_mod == NULL) {
		sys_log_e("input invalid parameters\n");
		while(1);
	}

	uart_pri = (uart_private_t *)uart_mod->private;

	switch ((uint32_t)(uart_pri->base)) {
#if defined (UART_1_MOD_BASE)
		case (uint32_t)UART_1_MOD_BASE:
			MX_USART1_UART_Init();
		break;
#endif
#if defined (UART_2_MOD_BASE)
		case (uint32_t)UART_2_MOD_BASE:
			MX_USART2_UART_Init();
		break;
#endif
		default :
			sys_log_e("invalid base addr\n");
		break;
	}
	/* enable receive interrupt */
	LL_USART_EnableIT_RXNE(uart_pri->base);
}

static int uart_register_callback(
	void *thiz,
	void (*call)(void *arg, uint8_t),
	void * func_para
) {
	module_t * uart_mod = (module_t *)thiz;
	uart_private_t * uart_pri;

	if (uart_mod == NULL) {
		return -1;
	}

	uart_pri = (uart_private_t *)uart_mod->private;

	uart_pri->call_back = call;
	uart_pri->fun_para = func_para;
	return 0;
}

static int uart_register_txdone_callback(
		void *thiz,
		void (*call) (void *arg),
		void *arg)
{
	module_t * uart_mod = (module_t *)thiz;

	uart_private_t * uart_pri;

	if (!uart_mod || !call)
		return -1;

	uart_pri = uart_mod->private;

	uart_pri->transmit_done_call = call;
	uart_pri->txdone_arg = arg;

	return 0;
}

void uart_dma_tx_done_handler(void *uart_base)
{
	uart_private_t *uart_pri;
	if (uart_base == NULL) {
		return ;
	}

	switch ((uint32_t) uart_base) {
#if defined (UART_1_MOD_BASE)
		case (uint32_t)UART_1_MOD_BASE:
			uart_pri = &uart_set[0].uart_private;
			uart_pri->transmit_done_call(uart_pri->txdone_arg);
		break;
#endif
		default :
		break;
	}

}

void irq_uart_user_handler(void * uart_base, uint8_t ch)
{
	uart_private_t *uart_pri;

	if (uart_base == NULL) {
		return ;
	}

	switch ((uint32_t) uart_base) {
#if defined (UART_1_MOD_BASE)
		case (uint32_t)UART_1_MOD_BASE:
			uart_pri = &uart_set[0].uart_private;
			uart_pri->call_back(uart_pri->fun_para, ch);
		break;
#endif
#if defined (UART_2_MOD_BASE)
		case (uint32_t)UART_2_MOD_BASE:
			uart_pri = &uart_set[1].uart_private;
			uart_pri->call_back(uart_pri->fun_para, ch);
		break;
#endif
		default :
		break;
	}
}

static int uart_write(void *thiz, uint8_t *data, uint16_t len)
{
	uart_private_t *uart_pri;
	module_t * uart_mod = (module_t *)thiz;

	if ((uart_mod == NULL) || (data == NULL)) {
		sys_log_e("Input invalid parameter!!\n");
		return -1;
	}

	uart_pri = (uart_private_t *)uart_mod->private;

	uart_raw_send(uart_pri->base, data, len);

	return 0;
}

static int uart_read(void *thiz, uint8_t *data)
{
	(void) thiz;
	(void) data;

	return 0;
}

static uint32_t uart_b_get_uart_baud(void *thiz)
{
	uart_private_t *uart_pri;
	module_t * uart_mod = (module_t *)thiz;

	if (uart_mod == NULL) {
		sys_log_e("Input invalid parameter!!\n");
		return -1;
	}

	uart_pri = (uart_private_t *)uart_mod->private;

	return get_uart_baud((void *)uart_pri->base);
}

static int uart_b_set_uart_baud(void *thiz, uint32_t baud)
{
	uart_private_t *uart_pri;
	module_t * uart_mod = (module_t *)thiz;

	if (uart_mod == NULL) {
		sys_log_e("input invliad parameter!!!\r\n");
		while(1);
	}

	uart_pri = (uart_private_t *)uart_mod->private;
	set_uart_baud(uart_pri->base, baud);

	return 0;
}

static int uart_bsp_set_parity(void *thiz, uint32_t parity)
{
	uart_private_t *uart_pri;
	module_t *uart_mod = (module_t *)thiz;

	if (!uart_mod) {
		sys_log_e("input invalid parameter!!!\r\n");
		while(1);
	}

	uart_pri = uart_mod->private;
	uart_set_parity(uart_pri->base, parity);
	return 0;
}

static uint32_t uart_bsp_get_parity(void *thiz)
{
	uart_private_t *uart_pri;
	module_t *uart_mod = (module_t *)thiz;

	if (!uart_mod) {
		sys_log_e("invalid parameter!!!\r\n");
		while(1);
	}

	uart_pri = uart_mod->private;
	return uart_get_parity(uart_pri->base);
}

static uart_ops_t uart_ops = {
	.init = uart_if_init,
	.register_rx_call = uart_register_callback,
	.register_txdone_call = uart_register_txdone_callback,
	.write = uart_write,
	.read =  uart_read,
	.get_baud = uart_b_get_uart_baud,
	.set_baud = uart_b_set_uart_baud,
	.set_parity = uart_bsp_set_parity,
	.get_parity = uart_bsp_get_parity,
};

static int uart_module_register(void)
{
	static int init_flag = 0;
	int i;

	if (init_flag) {
		return 0;
	}

	for (i = 0; i < (int)(sizeof(uart_set) / sizeof(uart_set[0])); i++) {
		uart_set[i].uart_mod.ops = (void *)&uart_ops;
		uart_set[i].uart_mod.private = (void *)&uart_set[i].uart_private;
		module_register(UART_1_MODULE + i, &uart_set[i].uart_mod);
	}

	init_flag = 0x01;
}

device_initcall(uart_module_register);

