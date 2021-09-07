#include "module.h"
#include <string.h>
#include "sci.h"

// modules list item header
static module_list		module_list_header;

int system_module_init(void)
{
	vListInitialise(&module_list_header);
	return 0;
}

int module_register(int flag, module_t *module)
{
	vListInitialiseItem(&module->list_item);
	listSET_LIST_ITEM_VALUE(&module->list_item, flag);
	listSET_LIST_ITEM_OWNER(&module->list_item, module);
	vListInsert(&module_list_header, &module->list_item);
	return 0;
}

module_t * module_get(int flag, const char *name)
{
	module_list *pxList = &module_list_header;
	module_t * pxModule = NULL;     
	module_t * pxNextmodule = NULL;     
	module_list_item * pxItem;

	if (listLIST_IS_EMPTY(pxList))
		goto exit;

	if (flag) {
		pxItem = (module_list_item *)&pxList->xListEnd;
		for (pxItem = pxItem->pxNext; pxItem != (module_list_item *)&(pxList->xListEnd); pxItem = pxItem->pxNext) {
			if (listGET_LIST_ITEM_VALUE(pxItem) == flag) {
				pxModule = (module_t *)listGET_LIST_ITEM_OWNER(pxItem);
				goto exit;
			}
		}
	}

	if (name) {
		pxItem = (module_list_item *)&pxList->xListEnd;
		for (pxItem = pxItem->pxNext; pxItem != (module_list_item *)&(pxList->xListEnd); pxItem = pxItem->pxNext) {
			pxNextmodule = (module_t *)listGET_LIST_ITEM_OWNER(pxItem);
			if (!(strcmp(name, pxNextmodule->name))) {
				pxModule = pxNextmodule;
				goto exit;
			}
		}

	}

exit:
	return pxModule;
}

extern uint32_t initcall_begin;
extern uint32_t initcall_end;

void all_module_register(void)
{
	initcall_t * p = (void *)&initcall_begin;

	for (; p < (void *)&initcall_end;){
		int (* fn)(void) = (void *)(*p++);
		if (fn)
			fn();
	}
}
