#ifndef __EXTI_BSP_H__
#define __EXTI_BSP_H__

#include "main.h"
#include "stm32f1xx.h"
#include "module.h"
#include "sci.h"

struct exti_ops_t {
	void (* init) (void *thiz);
	int (* read_exti) (void *thiz);
	int (* read_clear_exti) (void *thiz);
	int (* disable_exti_it) (void *thiz, uint16_t n);
	int (* enable_exti_it) (void *thiz, uint16_t n);
	int (* close_exit_nvic_it) (void *thiz,char);
	int (* call_back_register) (void *thiz, 
			uint32_t (* call_back) (void *arg, uint32_t exti_val), void *);
};

#endif
