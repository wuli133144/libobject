/**
 * @buffer RBuffer.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-01-13
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/io/RBuffer.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/thread.h>
#define DEFAULT_BUFFER_SIZE 1024

static int __construct(RBuffer *self,char *init_str)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    if (self->capacity == 0) {
        self->capacity  = DEFAULT_BUFFER_SIZE + 1 ;
    } else {
        self->capacity = (self->capacity + 1 + sizeof(int) - 1 )&(~(sizeof(int) - 1 )) ;
    }
    
    self->buffer = allocator_mem_alloc(allocator, self->capacity);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        return -1;
    }

    self->r_offset       = 0;
    self->w_offset       = 0;
    self->available_size = self->capacity - 1;
    self->used_size      = 0;
    self->ref_count      = 1;
    return 0;
}

static int __deconstrcut(RBuffer *self)
{
    allocator_t *allocator = ((Obj *)self)->allocator;

    dbg_str(EV_DETAIL,"buffer deconstruct,buffer addr:%p",self);
    allocator_mem_free(allocator, self->buffer);
    self->ref_count = 0;
    return 0;
}

static int __set(RBuffer *buffer, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        buffer->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        buffer->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        buffer->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        buffer->deconstruct = value;
    } else if (strcmp(attrib, "read") == 0) {
        buffer->read = value;
    } else if (strcmp(attrib, "write") == 0) {
        buffer->write = value;
    } else if (strcmp(attrib, "size") == 0) {
        buffer->size = value;
    } else if (strcmp(attrib, "is_empty") == 0) {
        buffer->is_empty = value;
    } else if (strcmp(attrib, "clear") == 0) {
        buffer->clear = value;
    } else if (strcmp(attrib, "move_unref") == 0) {
        buffer->move_unref = value;
    } else if (strcmp(attrib, "find") == 0) {
        buffer->find = value;
    } else if (strcmp(attrib, "has_used_size") == 0) {
        buffer->has_used_size = value;
    } else if (strcmp(attrib, "free_size") == 0) {
        buffer->free_size = value;
    } else if (strcmp(attrib, "copy") == 0) {
        buffer->copy = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        buffer->destroy = value;
    } else if (strcmp(attrib, "adapter_internal") == 0) {
        buffer->adapter_internal = value;
    } else if (strcmp(attrib, "expand_container") == 0) {
        buffer->expand_container = value;
    } else {
        dbg_str(EV_DETAIL,"buffer set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(RBuffer *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"buffer get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __size (RBuffer *self)
{
    return self->capacity -1;
}

static int __free_size(RBuffer *self)
{
    return self->available_size;
} 

static int __has_used_size(RBuffer *self)
{
    return self->used_size;
}

static int __is_empty(RBuffer *self)
{
    return self->used_size ? 0:1;
}

static int __clear(RBuffer *self)
{
    self->r_offset       = 0;
    self->w_offset       = 0;
    self->used_size      = 0;
    self->available_size = self->capacity -1 ;
    return 1;
}

//读指针 追 写指针

static  int __adapter_internal(RBuffer *self)
{
    if (self->r_offset < self->w_offset) {
        self->used_size = self->w_offset - self->r_offset;
    } else if (self->r_offset > self->w_offset ) {
        self->used_size = self->capacity - self->r_offset + self->w_offset;
    } else {
        self->used_size = 0;
    }

    self->available_size = self->capacity -1 - self->used_size;
    return 1;
}

static int __expand_container(RBuffer *self,int len)
{
    int available_write_size = self->available_size;
    int capacity = self->capacity ;
    void * tmp_buffer = NULL;
    if (len < available_write_size) {
        return 0;
    }

    len = (2*capacity + len + sizeof(int) - 1 )&(~(sizeof(int) -1 ));
    tmp_buffer = self->buffer;
    //allocator_mem_free(allocator,self->buffer);
    self->buffer = allocator_mem_alloc((self->parent.obj).allocator, len);
    if (self->buffer == NULL) {
        dbg_str(DBG_ERROR, "allocator_mem_alloc");
        self->buffer = tmp_buffer;
        return -1;
    }
    
    if (self->r_offset < self->w_offset){
        memcpy(self->buffer+self->r_offset,tmp_buffer+self->r_offset,self->used_size);
    } else if (self->w_offset < self->r_offset) {
        memcpy(self->buffer,tmp_buffer,self->w_offset);
        memcpy(self->buffer+self->r_offset,tmp_buffer+self->r_offset,self->capacity - self->r_offset);
    } else {
        
    }
    allocator_mem_free((self->parent.obj).allocator,tmp_buffer);

    self->capacity = len -1;
    self->available_size = self->capacity - self->used_size - 1;

    return 0;
}

static int __read(RBuffer *self, void *dst, int len)
{
   int  available_read_size = self->used_size;
   int  internal_end_size   = self->capacity - self->r_offset;

   if ( available_read_size == 0 || dst == NULL || len <= 0 ) {
       len = 0;
       return len;
   }
   //保证不会多读
   if ( len > available_read_size ) {
       len = available_read_size;
   }
   //两种情况
   //r < w 
   if (self->r_offset < self->w_offset) {
       memcpy(dst,self->buffer+self->r_offset,len);
       self->r_offset += len;
   // r > w
   } else if (self->r_offset > self->w_offset) {
       if (internal_end_size >= len ) {
           memcpy(dst,self->buffer+self->r_offset,len);
           self->r_offset += len;
       } else {
           memcpy(dst,self->buffer+self->r_offset,internal_end_size);
           self->r_offset += internal_end_size;
           memcpy(dst+internal_end_size,self->buffer,len - internal_end_size);
           self->r_offset = len - internal_end_size;
       }
   }
 
   self->adapter_internal(self);
   return len;
}

static int __write(RBuffer *self, void *src, int len)
{ 
    int available_write_size = self->available_size;
    int internal_end_size    = self->capacity - self->w_offset;
    int ret                  = 0;
    if (src == NULL || len <= 0 ) {
        len = 0;
        return len;
    }

    if (available_write_size < len ) {
        //extend space
        ret = self->expand_container(self,len);
        if (ret < 0) {
            dbg_str(DBG_ERROR,"rbuf extand space failed");
            return -1;
        }
    }

    available_write_size = self->available_size;
    internal_end_size    = self->capacity - self->w_offset;
    //保证不会多写
    if ( len > available_write_size ) {
       len = available_write_size;
    }
    //两种情况
    //r < w 
    if (self->r_offset <= self->w_offset ) {
       if (len <= internal_end_size) {
           memcpy(self->buffer+self->w_offset,src,len);
           self->w_offset += len;
       } else {
           memcpy(self->buffer+self->w_offset,src,internal_end_size);
           self->w_offset += internal_end_size;
           memcpy(self->buffer,src+internal_end_size,len - internal_end_size);
           self->w_offset = len - internal_end_size;
       }
    } else if (self->r_offset > self->w_offset) {
        memcpy(self->buffer+self->w_offset,src,len);
        self->w_offset += len;
    }
  
    self->adapter_internal(self);
    return len;
}

static  int __find(RBuffer *self,u_char c)
{
    int i;
    int internal_end_size;
    if (self->is_empty(self)) {
        return -1;
    }

    if (self->r_offset < self->w_offset) {
        for (i = self->r_offset ;i < self->w_offset;i++) {
             if (*((char*)self->buffer+i) == c) {
                 return i;
             }
        }
    } else if (self->r_offset > self->w_offset) {
        internal_end_size = self->capacity - self->r_offset;
        for (i = self->r_offset;i < self->capacity;i++) {
            if (*((char*)self->buffer+i) == c) {
                 return i;
            }
        }

        internal_end_size = self->used_size - internal_end_size;
        for (i = 0 ;i < internal_end_size;i++){
            if (*((char*)self->buffer+i) == c) {
                 return i;
             }
        }
    }
    return -1;
}

static void __destroy(RBuffer *self)
{
    if (--self->ref_count == 0){
        object_destroy(self);
    }
}

static int __copy_ref(RBuffer *self,RBuffer *dst)
{   
    if (dst == NULL||self == NULL ) {
        return -1;
    }
    dst = self;
    self->ref_count += 1;
    return 0;
}

static int __copy(RBuffer *self,RBuffer *dst,size_t len) 
{   
    int used_size      = self->has_used_size(self);
    int availible_size = self->free_size(self);
    int to_copy        = 0;
    int dst_free_size  = dst->free_size(dst);
    int write_to_end   = 0;

    if (len <= 0 || dst == NULL || used_size <= 0)
        return -1;
    to_copy = len < dst_free_size ? len:dst_free_size ;

    if (to_copy > used_size) {
        to_copy = used_size;
    }

    write_to_end = dst->w_offset > dst->r_offset ? 
                  (dst->capacity - dst->w_offset):(dst->r_offset - dst->w_offset);
    //1.实际右侧w_offset空余空间足够使用.直接拷贝              
    if (write_to_end >= to_copy) {
        if(dst->r_offset > dst->w_offset) {

            int to_end = self->capacity - self->r_offset;
            if (to_end >= to_copy) {
                memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_copy);
                dst->w_offset += to_copy;
            } else {
                memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_end);
                dst->w_offset += to_end;
                memcpy(dst->buffer+dst->w_offset,self->buffer,to_copy - to_end);
                dst->w_offset += to_copy - to_end;
            }
        } else {
            memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_copy);
            dst->w_offset += to_copy;
        }
    } else {
        //分两种情况
        //1.dst->w > dst->r
        if (dst->w_offset > dst->r_offset) {
            if (self->r_offset < self->w_offset) {
                int to_end = dst->capacity - dst->w_offset;
                memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_end);
                dst->w_offset += to_end;
                memcpy(dst->buffer,self->buffer+self->r_offset+to_end,to_copy- to_end);
                dst->w_offset = to_copy-to_end;
            } else {
                int to_src_end = self->capacity - self->r_offset;
                int to_end     = dst->capacity  - dst->w_offset;
                if (to_copy <= to_src_end) {
                    memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_end);
                    dst->w_offset += to_end;
                    memcpy(dst->buffer,self->buffer+self->r_offset+to_end,to_copy-to_end);
                    dst->w_offset = to_copy - to_end;
                } else {
                    if (to_src_end < to_end) {
                          memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_src_end);
                          dst->w_offset += to_src_end;
                          int tmp_end = to_end - dst->w_offset;
                          memcpy(dst->buffer+ dst->w_offset,self->buffer,tmp_end);
                          memcpy(dst->buffer,self->buffer+tmp_end,to_copy-to_src_end-tmp_end);
                          dst->w_offset = to_copy - to_src_end - tmp_end;  
                    } else {
                        memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_end);
                        dst->w_offset += to_end;
                        int tmp_src_end = self->capacity - (self->r_offset + to_end);
                        memcpy(dst->buffer,self->buffer+self->r_offset+to_end,tmp_src_end);
                        dst->w_offset = tmp_src_end;
                        memcpy(dst->buffer+self->w_offset,self->buffer,to_copy - (self->capacity - self->r_offset));
                        dst->buffer += to_copy - (self->capacity - self->r_offset) ;
                    }
                }
            }
        }else {
             if (self->r_offset < self->w_offset) {
                 memcpy(dst->buffer+self->w_offset,self->buffer+self->r_offset,to_copy);
                 dst->w_offset += to_copy;
             } else {
                 int to_src_end = self->capacity - self->r_offset;
                 if (to_src_end >= to_copy) {
                     memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_copy);
                     dst->w_offset += to_copy;
                 } else {
                     memcpy(dst->buffer+dst->w_offset,self->buffer+self->r_offset,to_src_end);
                     dst->w_offset += to_src_end;
                     memcpy(dst->buffer+dst->w_offset,self->buffer,to_copy - to_src_end);
                     dst->w_offset += to_copy - to_src_end;
                 }
             }
        }
    }
    //update
    dst->adapter_internal(dst);
    return 0;
}

static int __move_unref(RBuffer *self,RBuffer*dst,size_t len)
{
    return -1;
}

static class_info_entry_t buffer_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Stream","parent",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","read", __read,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","write", __write,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","size", __size,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","clear", __clear,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER,"","is_empty", __is_empty,sizeof(void *)},
    [10] = {ENTRY_TYPE_VFUNC_POINTER,"","copy_ref", __copy_ref,sizeof(void *)},
    [11] = {ENTRY_TYPE_VFUNC_POINTER,"","destroy", __destroy,sizeof(void *)},
    [12] = {ENTRY_TYPE_VFUNC_POINTER,"","move_unref", __move_unref,sizeof(void *)},
    [13] = {ENTRY_TYPE_VFUNC_POINTER,"","copy", __copy,sizeof(void *)},
    [14] = {ENTRY_TYPE_VFUNC_POINTER,"","expand_container", __expand_container,sizeof(void *)},
    [15] = {ENTRY_TYPE_VFUNC_POINTER,"","adapter_internal", __adapter_internal,sizeof(void *)},
    [16] = {ENTRY_TYPE_VFUNC_POINTER,"","has_used_size", __has_used_size,sizeof(void *)},
    [17] = {ENTRY_TYPE_VFUNC_POINTER,"","free_size", __free_size,sizeof(void *)},
    [18] = {ENTRY_TYPE_VFUNC_POINTER,"","find", __find,sizeof(void *)},
    [19] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("RBuffer",buffer_class_info);

static int Test_buffer(TEST_ENTRY *entry)
{
    RBuffer *buffer;
    allocator_t *allocator = allocator_get_default_alloc();
    char in[9] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'};
    char out[9];
    int len;
    char *p = "fdasfasd";
    char dst[100]={0};

    buffer = OBJECT_NEW(allocator, RBuffer, NULL);
    
    len=buffer->write(buffer,p,strlen(p));
    
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

   len=buffer->write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    len=buffer->write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);
  
    
    buffer->read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);
    
    buffer->read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    memset(dst,0,100);
    buffer->read(buffer,dst,strlen(p));

    dbg_str(DBG_SUC,"current buffer state: dst str:%s used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                dst,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    
    len=buffer->write(buffer,p,strlen(p));
   dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    len=buffer->write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    
    len=buffer->write(buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                buffer->has_used_size(buffer),
                buffer->free_size(buffer),
                buffer->r_offset,
                buffer->w_offset);

    object_destroy(buffer);

    return 1;
}
REGISTER_TEST_FUNC(Test_buffer);

static void* ringproducer(void * arg)
{
    RBuffer * buffer = (RBuffer *)arg;
    char * p ="pruducer ";
    int size ;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->write(buffer,p,strlen(p));
        
        if (size){
            dbg_str(DBG_SUC,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->has_used_size(buffer),
                    buffer->free_size(buffer));
            usleep(200000);
        }
        else {
            dbg_str(DBG_ERROR,"producer buffer is full");
            dbg_str(DBG_ERROR,"producer buffer current producer size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->has_used_size(buffer),
                    buffer->free_size(buffer));
            usleep(1000000);
        }
    }
    return NULL;
}

static void * ringconsumer(void *arg)
{
    RBuffer * buffer = (RBuffer *)arg;
    char tmp[1024] = {0};
    char * p ="pruducer ";
    int size;
    if (arg == NULL) {
        return NULL;
    }
    //DO
    while (1) {
        size = buffer->read(buffer,tmp,sizeof(tmp)/sizeof(char));
        //usleep(10000);
        if (size){
            dbg_str(DBG_SUC,"consume buffer:%s",tmp);
            dbg_str(DBG_SUC,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->has_used_size(buffer),
                    buffer->free_size(buffer));

            memset(tmp,0,100);
        }  else {
            dbg_str(DBG_ERROR,"consume ringbuffer is nil ");
            dbg_str(DBG_ERROR,"consume buffer current consume size:%d  used_size:%d avaliable_size:%d",
                    size,
                    buffer->has_used_size(buffer),
                    buffer->free_size(buffer));

            usleep(1000000);
        }
    }
    return NULL;
}

static int test_rbuf_producer_consumer()
{
    allocator_t * allocator = allocator_get_default_alloc();
    RBuffer * buffer =   OBJECT_NEW(allocator,RBuffer ,NULL);
    
    Thread * producer = OBJECT_NEW(allocator,Thread ,NULL);
    producer->set_start_arg(producer,buffer);
    producer->set_start_routine(producer,ringproducer);
    producer->start(producer);
    producer->detach(producer);
    
    Thread * consumer = OBJECT_NEW(allocator,Thread ,NULL);
    consumer->set_start_arg(consumer,buffer);
    consumer->set_start_routine(consumer,ringconsumer);
    consumer->start(consumer);
    consumer->detach(consumer);
    pause();
    object_destroy(buffer);
    object_destroy(producer);
    object_destroy(consumer);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_rbuf_producer_consumer);

static int test_rbuf_copy(TEST_ENTRY * entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    RBuffer *in_buffer ,*out_buffer = NULL;
    int len;
    char *p = "fdasfasd";
    char dst[100]={0};

    in_buffer = OBJECT_NEW(allocator, RBuffer, NULL);
    len = in_buffer->write(in_buffer,p,strlen(p));
    
    //before copy 
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
            len,
            in_buffer->has_used_size(in_buffer),
            in_buffer->free_size(in_buffer),
            in_buffer->r_offset,
            in_buffer->w_offset);

    out_buffer = OBJECT_NEW(allocator, RBuffer, NULL);
    in_buffer->copy(in_buffer,out_buffer,2048);

    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);

    in_buffer->copy(in_buffer,out_buffer,2048);
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    
    in_buffer->copy(in_buffer,out_buffer,2048);
    dbg_str(DBG_SUC,"current buffer state: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    
    char buf[100] = {0};
    memset(dst,0,100);
    out_buffer->read(out_buffer,dst,strlen(p));
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);

    in_buffer->copy(in_buffer,out_buffer,2048);
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    
    dbg_str(DBG_ERROR,"#######################################");
    in_buffer->copy(in_buffer,out_buffer,2048);
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    dbg_str(DBG_ERROR,"#######################################");

    
    memset(dst,0,100);
    out_buffer->read(out_buffer,dst,strlen(p));
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    
    dbg_str(DBG_ERROR,"#######################################");

    memset(dst,0,100);
    out_buffer->read(out_buffer,dst,strlen(p));
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    
    dbg_str(DBG_ERROR,"#######################################");

    memset(dst,0,100);
    out_buffer->read(out_buffer,dst,strlen(p));
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                out_buffer->has_used_size(out_buffer),
                out_buffer->free_size(out_buffer),
                out_buffer->r_offset,
                out_buffer->w_offset);
    object_destroy(in_buffer);
    object_destroy(out_buffer);
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_rbuf_copy);

static int test_rbuf_extend_space(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_alloc();
    RBuffer *in_buffer ,*out_buffer = NULL;
    int len;
    char *p = "fdasfasdFASDFFASD";
    char dst[100]={0};

    in_buffer = OBJECT_NEW(allocator, RBuffer, NULL);
    len = in_buffer->write(in_buffer,p,strlen(p));
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                in_buffer->has_used_size(in_buffer),
                in_buffer->free_size(in_buffer),
                in_buffer->r_offset,
                in_buffer->w_offset);
    
    dbg_str(DBG_ERROR,"#######################################");

    
    dbg_str(DBG_SUC,"current out_buffer: writed_size:%d used_size:%d avaiable_size:%d r_offset:%d w_offset:%d",
                len,
                in_buffer->has_used_size(in_buffer),
                in_buffer->free_size(in_buffer),
                in_buffer->r_offset,
                in_buffer->w_offset);

    object_destroy(in_buffer);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_rbuf_extend_space);
