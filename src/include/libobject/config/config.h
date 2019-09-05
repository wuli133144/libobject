#ifndef __CONFIG_H__H
#define __CONFIG_H__H

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/vector.h>
#include <libobject/json/json.h>

typedef struct config_s Config;

struct config_s{
	Obj obj;
PUBLIC
	int (*construct)(Config *config,char *init_str);
	int (*deconstruct)(Config *config);
	int (*set)(Config *config, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    int (*load)(Config *conf,const char *path);

PRIVITE
    json_object* (*parsejson)(Config *conf);
    json_object * json;
    char filepath[1024];
};

#endif
