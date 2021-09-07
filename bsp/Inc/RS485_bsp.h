#ifndef		__RS485_BSP_H__
#define		__RS485_BSP_H__

#include "sci.h"
#include "module.h"
#include "stm32f103xb.h"

#define MAX_RS485_IF		2

struct RS485_ops {
	void (*init) (void *thiz);
	void (*write) (void *thiz, uint8_t * data, uint16_t len);
	/* interrupt mode */
	int (*register_call) (void *thiz,
			void (*call) (void *, uint8_t),
			void *arg);
	uint32_t (* get_baud) (void *thiz);
	int (*set_baud) (void *thiz, uint32_t baud);
	int (*set_parity)(void *thiz, uint32_t parity);
	uint32_t (*get_parity)(void *thiz);
};

struct rs485_gpio_if_t{
	module_t * gpio_inst;
	uint32_t gpio_em;
};

struct rs485_uart_if_t {
	char * name;
	module_t * uart_inst;
};


typedef struct {
	struct rs485_gpio_if_t gpio_inst;
	struct rs485_uart_if_t uart_inst;
}rs485_mod_private_t;

#endif

