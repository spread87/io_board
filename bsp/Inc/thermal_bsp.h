#ifndef __THERMAL_BSP_H__
#define __THERMAL_BSP_H__

typedef struct {
	void (*init) (void *thiz);
	int (*read_temp) (void *thiz, int type);
}thm_ops_t;

enum {
	TEMP1 = 0,
	TEMP2,
	TEMP3,
	TEMP4,
	TEMPMAX
};

#endif
