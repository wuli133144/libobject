#ifndef __LIST_LINKED_H__
#define __LIST_LINKED_H__

#include <stdio.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/core/list.h>
#include <libobject/utils/data_structure/link_list.h>
#include <libobject/core/iterator_linkedlist.h>

typedef struct Linked_List_s Linked_List;

struct Linked_List_s{
	List list;

	int (*construct)(List *list,char *init_str);
	int (*deconstruct)(List *list);
	int (*set)(List *list, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*add)(List *list,Iterator *iter, void *value);
    int (*add_back)(List *list,void *value);
    int (*delete)(List *list,Iterator *iter);
    int (*remove)(List *list,Iterator *iter, void **data);
    int (*remove_back)(List *list, void **data);
    int (*detach_front)(List *list,Iterator *iter);
    int (*free_detached)(List *list,Iterator *iter);
    void (*for_each)(List *list,void (*func)(Iterator *iter));
    Iterator *(*begin)(List *list);
    Iterator *(*end)(List *list);
    int (*destroy)(List *list);


#define MAX_NAME_LEN 50
    char name[MAX_NAME_LEN];
#undef MAX_NAME_LEN
    llist_t *llist;
};

#endif
