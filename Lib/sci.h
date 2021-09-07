#ifndef __SCI_H__
#define __SCI_H__

#include <stdio.h>
#include "stm32f1xx.h"

typedef int (*initcall_t) (void);
#define ____define_initcall(fn, id) \
	initcall_t __initcall_##id##_##fn = (void *)fn;

#define __define_initcall(fn, id) ____define_initcall(fn, id)

#define device_initcall(fn) __define_initcall(fn, 6)

#define _sys_log_(arg,...)	do{\
	printf(arg,##__VA_ARGS__);\
	}while(0)

#define sys_log		_sys_log_

#define sys_log_e(arg,...)	sys_log("[e] cur func:%s,cur line:%d "arg,\
		__func__,__LINE__,##__VA_ARGS__)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))
void udelay(uint32_t xus);
void mdelay(uint32_t xms);
#endif
