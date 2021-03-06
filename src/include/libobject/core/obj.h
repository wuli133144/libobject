#ifndef __OBJ_H__
#define __OBJ_H__

#include <libobject/core/class_deamon.h>
#include <libobject/core/object.h>

typedef struct obj_s Obj;
struct obj_s{
	allocator_t *allocator;
	int (*construct)(Obj *obj,char *init_str);
	int (*deconstruct)(Obj *obj);
	int (*set)(Obj *obj, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
#define MAX_CLASS_NAME_LEN 20
    char name[MAX_CLASS_NAME_LEN];
#undef MAX_CLASS_NAME_LEN
};

#endif
