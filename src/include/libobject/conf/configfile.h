#ifndef ___CONFIG_FILE_H 
#define ___CONFIG_FILE_H 

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/core/rbtree_map.h>
#include <libobject/core/string.h>


typedef struct s_config_t    ConfigFile;

struct s_config_t {
    Obj    obj;

    int (*construct)(ConfigFile *,char *init_str);
	int (*deconstruct)(ConfigFile *);
	int (*set)(ConfigFile *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);
    
    int (*parse_config_internal)(ConfigFile *,void *buf);
    int (*load_file)(ConfigFile *,const char * file);
    int (*get_value)(ConfigFile *,void *key,void **ele);
    /*attribute*/
    Map  * map;

};

#endif 