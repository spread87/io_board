#ifndef __UART_BSP_H__
#define __UART_BSP_H__

#include "cmsis_os.h"

typedef struct {
	void (*init) (void *thiz);
	int (*register_rx_call) (void *thiz,
		void (*call) (void *,uint8_t),
		void *arg);
	int (*register_txdone_call) (void *thiz,
		void (*call) (void *),
		void *done_arg);
	int (* write) (void *thiz, uint8_t *data, uint16_t len);
	int (* read) (void *thiz, uint8_t *data);
	uint32_t (*get_baud) (void *thiz);
	int (* set_baud) (void *thiz, uint32_t);
	int (* set_parity) (void *thiz, uint32_t);
	uint32_t (* get_parity) (void *thiz);
}uart_ops_t;

typedef struct uart_private {
	void * base;
	void * fun_para;
	void (* call_back) (void *arg, uint8_t );
	void (* transmit_done_call) (void *arg);
	void *txdone_arg;
}uart_private_t;

#endif

