#ifndef __TIMER_BSP_H__
#define __TIMER_BSP_H__

#include "cmsis_os.h"

typedef struct timer_ops {
	void (*init) (void *thiz); 
	int (*register_call) (void *thiz, 
		void (*tim_call)(void *),
		void * arg);
	int (*start) (void *thiz);
	int (*stop) (void *thiz);
	void (*reload)(void *thiz, uint32_t, uint32_t);
	int (*create_timer)(void *thiz, uint32_t reload, void (*call)(void *),void *arg, uint32_t start);
	void (*enable_int)(void *thiz, uint32_t);

}timer_ops_t;

typedef struct timer_private {
	void * base;
	void * fun_para;
	void (*call_back) (void *arg);
}timer_private_t;


#endif

