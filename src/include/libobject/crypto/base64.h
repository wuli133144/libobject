#ifndef __BASE64_H
#define __BASE64_H

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

typedef  struct base64_t Base64;

struct base64_t {
    Obj obj;

	int (*construct)(Base64 *,char *init_str);
	int (*deconstruct)(Base64 *);
	int (*set)(Base64 *, char *attrib, void *value);
    void *(*get)(void *obj, char *attrib);

    /*attributes*/
    int (*base64_encode)(Base64 *,char *,int ,char *);
    int (*base64_decode)(Base64 *,char *,int ,char *);

};

#endif 