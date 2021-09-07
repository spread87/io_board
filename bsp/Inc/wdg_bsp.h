#ifndef __WDG_BSP_H__
#define __WDG_BSP_H__

#include "sci.h"
#include "module.h"

struct wdg_ops_t {
	void (*init) (void *thiz);
	void (*feed_dog) (void *thiz);
};
#endif

