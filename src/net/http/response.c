/**
 * @file Response.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-01-01
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
#include <assert.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/http/Response.h>

static int __construct(Response *response,char *init_str)
{
    allocator_t *allocator = response->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"response construct, response addr:%p",response);
    response->buffer = OBJECT_NEW(response->obj.allocator,RBuffer,NULL); 
    response->response_context = OBJECT_NEW(allocator,String,NULL);
    response->current_size  = 0;
    response->content_length = 0;
    response->code = HTTP_CODE_OK;
    return 0;
}

static int __deconstrcut(Response *response)
{
    dbg_str(DBG_DETAIL,"response deconstruct,response addr:%p",response);
    object_destroy(response->buffer);
    if (response->response_context) {
        object_destroy(response->response_context);
        response->response_context = NULL;
    }
    return 0;
}

static int __set(Response *response, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        response->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        response->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        response->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        response->deconstruct = value;
    } else if (strcmp(attrib, "set_buffer") == 0) {
        response->set_buffer = value;
    } else if (strcmp(attrib, "read") == 0) {
        response->read = value;
    } else if (strcmp(attrib, "response_parse") == 0) {
        response->response_parse = value;
    } else if (strcmp(attrib, "parse_response_internal") == 0) {
        response->parse_response_internal = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"response set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Response *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"response get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __set_buffer(Response *response, RBuffer *buffer)
{
    response->buffer = buffer;

    return 0;
}

static HTTP_CODE_T __get_http_code(int code)
{
    HTTP_CODE_T ret ;
    assert(code > 0);
    switch (code)
    {
        case 200:
            /* code */
            ret = HTTP_CODE_OK;
            break;
        case 301:
            ret = HTTP_CODE_REMOVED;
            break;
        case 400:
            ret = HTTP_CODE_BAD_REQUEST;
            break;
        case 401:
            ret = HTTP_CODE_NO_PERMISSTION;
            break;
        case 403:
            ret = HTTP_CODE_FORBIDDEN;
            break;
        case 500:
            ret = HTTP_CODE_SERVER_ERROR;
            break;
        case 502:
            ret = HTTP_CODE_BAD_NETGATE;
            break;
        case 503:
            ret = HTTP_CODE_BAD_SERVER;
            break;
        case 505:
            ret = HTTP_CODE_NO_SUPPORT;
            break;
        default:
            dbg_str(DBG_ERROR,"Unable to support response code = %d ",code);
            break;
    }
    return ret;
}

// static int __read_line_internal(char * dest,int dest_len,char * src)
// {
//     int ret ,i,len = 0;
//     assert(dest != NULL&&src != NULL && dest_len);
//     len = strlen(src);
//     len = len > dest_len ? dest_len:len;

//     for(i  = 0;i< len; i++) {
//         if (src[i] == '\n') {
//             ret = i;
//             break;
//         }
//     }
    
//     if (i >= len) {
//         ret = -1;
//         goto end;
//     }

//     i+=1;

//     dest_len = i > dest_len ? dest_len:i;
//     memcpy(dest,src,dest_len);
//     dest[dest_len - 1] = '\0';
//     ret = 0;
// end:
//     return ret;
    
// }

static int __read_line_internal(char * dest,int dest_len,char * src,int len)
{
    int ret ,i = 0;
    char * p,*q = NULL;
    int pos = 0;
    assert(dest != NULL&&src != NULL && dest_len && len);
    q = src;
    len = strlen(src);
    len = len > dest_len ? dest_len:len;

    p = strstr(src,"\r\n");
    i = p - q;
    if (i > dest_len - 1) {
        ret = -1;
        goto end;
    }

    memcpy(dest,q,i);
    dest[i] = '\0';

    ret = 0;
end:
    return ret;
    
}

static  char *  __lremove_space(char *p)
{   
    char * q = p;
    int i ,len= 0;
    assert(p != NULL);
    len = strlen(p);

    for (i = 0; i < len ;i++) {
        if (p[i] == ' ') {
            p = p + 1;
        }else {
            break;
        }
    }
    return p;
}

static  char *  __rremove_space(char *p)
{   
    char * q = p;
    int i ,len= 0;
    assert(p != NULL);
    len = strlen(p);

    for (i = len -1; i > 0  ;i--) {
        if (p[i] == ' ') {
            p[i] = '\0';
        }else {
            break;
        }
    }
    return p;
}

static int __parse_code_internal(char *buf,HTTP_CODE_T * code)
{
    int ret ,pos  = 0;
    char * p ,*q  = buf;
    char buff[10] = {0};

    int len = strlen(buf);
    assert(len);

    p = strchr(buf,' ');
    if (p == NULL) {
        ret = -1;
        goto end;
    }

    p = __lremove_space(p);
    p = __rremove_space(p);
    q = strchr(p,' ');
    len = q - p;
    strncpy(buff,p,len);
    ret = atoi(buff);
    *code = __get_http_code(ret);

end:
    return ret;
}

static int __get_http_context_len(char *buffer,int len)
{
    int ret ,i,pos = 0;
    char * p,*q = NULL;
    char * end = NULL;
    char tmpbuf[10] = {0};

    assert(buffer && len);
    end = buffer + len -1;

    p = strstr(buffer,"Content-Length:");
    if (p == NULL) {
        ret = -1;
        goto end;
    }

    p = p + 15;
    if (p > end) {
        ret = -2;
        goto end;
    }

    p = __lremove_space(p);
    //q = strchr(p,'\n');
    q = strstr(p,"\r\n");
    i = q - p;

    memcpy(tmpbuf,p,i);
    __rremove_space(tmpbuf);
    __lremove_space(tmpbuf);

    ret = atoi(tmpbuf);
end:
    return ret;
}

static int __response_parse(Response *response,void *buffer,int len)
{
   
    assert(buffer && len);
    HTTP_CODE_T code = HTTP_CODE_OK;
    RBuffer *rbuffer = response->buffer;
    int ret = 0;

    if (len) {
        rbuffer->write(rbuffer,buffer,len);
    }

    if (!response->content_length) {
       #if 1
       char buffline[1024] = {0};

       ret = __read_line_internal(buffline,1024,buffer,len);
       if (ret < 0) {
           dbg_str(DBG_ERROR,"read_line_internal error %d ",ret);
           goto end;
       }
       ret = __parse_code_internal(buffline,&code);
       if (ret < 0) {
           dbg_str(DBG_ERROR,"parse_code_internal error %d ",ret);
           goto end;
       }
       response->code = code;
       ret = __get_http_context_len(buffer,len);
       if (ret < 0) {
           dbg_str(DBG_ERROR,"get_context_len error %d ",ret);
           goto end;
       }
       response->content_length = ret;
       #endif 
    }
end:
    response->current_size = rbuffer->has_used_size(rbuffer);
    return ret;
}

static int __parse_response_internal(Response *response)
{
    int pos;
    int size;
    char * buffer = NULL,*find_pos = NULL;
    allocator_t *allocator = response->obj.allocator;

    size = response->buffer->has_used_size(response->buffer);
    buffer = allocator_mem_alloc(allocator,size);
    memset(buffer,0,size);

    response->buffer->read(response->buffer,buffer,size);
    find_pos = strstr(buffer,"\r\n\r\n");
    if (find_pos < 0) {
        goto end;
    }

    pos = find_pos - buffer;

    response->response_context->clear(response->response_context);
    response->response_context->assign(response->response_context,buffer+pos);

end:
    
    response->buffer->clear(response->buffer);
    allocator_mem_free(allocator,buffer);
    buffer = NULL;
    return pos ;
}

static int __read(Response *response)
{
    dbg_str(DBG_SUC, "read response");
    return 1;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","set_buffer",__set_buffer,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER,"","read",__read,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER,"","response_parse",__response_parse,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER,"","parse_response_internal",__parse_response_internal,sizeof(void *)},
    [9 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Response",concurent_class_info);

int test_response()
{
    Response *response;
    allocator_t *allocator = allocator_get_default_alloc();

    response = OBJECT_NEW(allocator, Response, NULL);

    object_destroy(response);
}
REGISTER_STANDALONE_TEST_FUNC(test_response);
