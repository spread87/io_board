#ifndef __CURVOL_BSP_H__
#define __CURVOL_BSP_H__

typedef struct {
	void (*init) (void *thiz);
	int (*read) (void *thiz, int type);
	int (*auto_cali) (void *thiz, int type);
	int (*set_para) (void *thiz, int type, int val);
}elec_ops_t;

enum {
	CURRENT1 = 0,
	CURRENT2,
	VOLTAGE,
	ELECMAX
};

#endif
