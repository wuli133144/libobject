#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>
#include <libobject/io/RBuffer.h>
#include <libobject/core/string.h>

//http 常用返回返回码
typedef enum HTTP_CODE_E
{
    HTTP_CODE_OK               = 200,  //请求成功
    HTTP_CODE_REMOVED          = 301,  //永久性移除
    HTTP_CODE_BAD_REQUEST      = 400,  //服务器不理解请求格式
    HTTP_CODE_NO_PERMISSTION   = 401,
    HTTP_CODE_FORBIDDEN        = 403,
    HTTP_CODE_SERVER_ERROR     = 500,  //服务器内部错误
    HTTP_CODE_BAD_NETGATE      = 502,
    HTTP_CODE_BAD_SERVER       = 503,
    HTTP_CODE_NO_SUPPORT       = 505

}HTTP_CODE_T;

typedef struct response_s Response;

struct response_s{
	Obj obj;

	int (*construct)(Response *,char *init_str);
	int (*deconstruct)(Response *);
	int (*set)(Response *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

	/*virtual methods reimplement*/
    int (*set_buffer)(Response *response, RBuffer *buffer);
    int (*read)(Response *response);
    int (*response_parse)(Response *response,void *buffer,int len);
    int (*parse_response_internal)(Response *);
    int current_size;
    int content_length;
    RBuffer *buffer;
    HTTP_CODE_T code;
    String * response_context;
};

#endif
