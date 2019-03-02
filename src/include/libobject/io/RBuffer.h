#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/Stream.h>

typedef struct buffer_s RBuffer;

enum buffer_operation_flag_s{
    BUFFER_WRITE_OPERATION = 1,
    BUFFER_READ_OPERATION,
};

typedef enum rbuf_memory_model {
    RBUF_MEMORY_STATIC = 1, //静态内存
    RBUF_MEMORY_DYNAMIC ,   //动态内存调整
}rbuf_memory_model_t;

struct buffer_s{
	Stream parent;

	int (*construct)(RBuffer *,char *init_str);
	int (*deconstruct)(RBuffer *);
	int (*set)(RBuffer *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*read)(RBuffer*, void *dst, int len);
    int (*write)(RBuffer *, void *src, int len);
    int (*size)(RBuffer *);
    int (*is_empty)(RBuffer*);
    /*int (*printf)(RBuffer *buffer, const char *fmt, ...);
     *int (*memcopy)(RBuffer *buffer, void *addr, int len);
     */
    void (*clear)(RBuffer*);
    int  (*move_unref)(RBuffer *,RBuffer*,size_t len);
    int  (*copy_ref)(RBuffer*,RBuffer *);
    int  (*copy)(RBuffer*,RBuffer *,size_t len);
    int  (*find)(RBuffer*,u_char c);
    void (*destroy)(RBuffer*);
    int  (*has_used_size)(RBuffer*);
    int  (*free_size)(RBuffer*);
    
    void (*set_model)(RBuffer *,rbuf_memory_model_t);
    void (*adapter_internal)(RBuffer*);
    int  (*expand_container)(RBuffer*,int len);
    /*attribs*/
    void   *buffer;
    int    r_offset;
    int    w_offset; 
    int    used_size;      //已经使用的空间
    int    available_size; //未使用的空间
    int    capacity;       //总大小
    size_t ref_count;
    uint8_t last_operation_flag;
    rbuf_memory_model_t model; //内存模型
};

#endif
