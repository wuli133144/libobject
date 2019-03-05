/**
 * @file Base64.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-10-24
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
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/crypto/base64.h>

static int __construct(Base64 *base, char *init_str)
{
    dbg_str(NET_DETAIL, "base construct, base addr:%p", base);

    return 0;
}

static int __deconstrcut(Base64 *base)
{
    dbg_str(NET_DETAIL, "base deconstruct, base addr:%p", base);

    return 0;
}

static int __set(Base64 *base, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        base->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        base->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        base->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        base->deconstruct = value;
    } else if (strcmp(attrib, "base64_decode") == 0) {
        base->base64_decode = value;
    } else if (strcmp(attrib, "base64_encode") == 0) {
        base->base64_encode = value;
    }
    else {
        dbg_str(NET_DETAIL, "base set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Base64 *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "base get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

/*
 BIO *bio, *b64;
 char message[] = "Hello World \n";

 b64 = BIO_new(BIO_f_base64());
 bio = BIO_new_fp(stdout, BIO_NOCLOSE);
 BIO_push(b64, bio);
 BIO_write(b64, message, strlen(message));
 BIO_flush(b64);

 BIO_free_all(b64);

*/
static int __base64_encode(Base64 *base,char *str,int str_len,char *encode){
    int ret = 0;
    BIO *bmem,*b64;
    BUF_MEM *bptr;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64,bmem);
    BIO_write(b64,str,str_len); //encode
    BIO_flush(b64);
    BIO_get_mem_ptr(b64,&bptr);
    
    ret = bptr->length;
    memcpy(encode,bptr->data,bptr->length);
    BIO_free_all(b64);
    return ret;
}

static int __base64_decode(Base64 *base,char *str,int str_len,char *decode){
    int len = 0;
    BIO *b64,*bmem;
    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new_mem_buf(str,str_len);
    bmem = BIO_push(b64,bmem);
    len = BIO_read(bmem,decode,str_len);
    decode[len] = '\0';
    BIO_free_all(bmem);
    return len;
}

static class_info_entry_t base64_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "base64_decode", __base64_decode, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "base64_encode", __base64_encode, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Base64", base64_class_info);

static int test_base64_encode(TEST_ENTRY * entry)
{
    allocator_t * allocator = allocator_get_default_alloc();
    Base64 * base = OBJECT_NEW(allocator,Base64,NULL);

    char input[] = "123456";
    int len = strlen(input);
    char buffer[100] = {0};

    base->base64_encode(base,input,len,buffer);
    
    dbg_str(DBG_SUC,"%s",buffer);


    object_destroy(base);
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_base64_encode);

static int test_base64_decode(TEST_ENTRY * entry)
{
    allocator_t * allocator = allocator_get_default_alloc();
    Base64 * base = OBJECT_NEW(allocator,Base64,NULL);

    char input[] = "MTIzNDU2\n";
    int len = strlen(input);
    char buffer[100] = {0};

    base->base64_decode(base,input,len,buffer);
    
    dbg_str(DBG_SUC,"%s",buffer);


    object_destroy(base);
    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_base64_decode);