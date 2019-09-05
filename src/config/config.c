/**
 * @file Config.c
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
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/dbg/debug_businesses.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/config/config.h>

static int __construct(Config *config, char *init_str)
{
    dbg_str(CONFIG_DETAIL, "Config construct, Config addr:%p", config);
    config->json = NULL;
    //config->filepath = {0};
    memset(config->filepath,0,sizeof(config->filepath)/sizeof(config->filepath[0]));
    return 0;
}

static int __deconstrcut(Config *config)
{
    dbg_str(CONFIG_DETAIL, "Config deconstruct, Config addr:%p", config);
    if (config->json) {
        json_object_put(config->json);
        config->json = NULL;
    }
    memset(config->filepath,0,sizeof(config->filepath)/sizeof(config->filepath[0]));
    return 0;
}

static int __set(Config *config, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        config->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        config->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        config->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        config->deconstruct = value;
    } else if (strcmp(attrib, "load") == 0) {
        config->load = value;
    } else if (strcmp(attrib, "parsejson") == 0) {
        config->parsejson = value;
    } 
    else {
        dbg_str(CONFIG_DETAIL, "Config set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Config *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(CONFIG_WARNNING, "Config get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static inline int checkfile(const char *file) 
{
    int i = 0;
    int size = strlen(file);
    assert(size > 5);
    if (file[size-1]=='n' &&
        file[size-2]=='o' &&
        file[size-3]=='s' &&
        file[size-4]=='j' &&
        file[size-5]=='.') {
            return 1;
        }
    return 0;
}

static int load(Config *obj,const char *file) 
{
    int size = -1;
    if (!file) {
        dbg_str(CONFIG_ERROR,"Config load failed config file path unvalid!");
        return -1;
    }
    size = strlen(file);
    if (size > 1024) {
        dbg_str(CONFIG_ERROR,"config file path'length too big. not support!");
        return -1; 
    }

    if (!checkfile(file))  {
        dbg_str(CONFIG_ERROR,"config file format unvalid");
        return -1; 
    }

    strncpy(obj->filepath,file,size);
    obj->json = obj->parsejson(obj);
    if (!obj->json) {
        dbg_str(CONFIG_ERROR,"config PARSE JSON failed");
        return -1; 
    }
    return 0;
}

static json_object * parsejson(Config * conf) 
{
    json_object * pjson = NULL;
    char *pfile = conf->filepath;
    if (!pfile) {
        return NULL;
    }

    pjson = json_object_from_file(pfile);
    if (!pjson) {
        dbg_str(CONFIG_ERROR,"config PARSE JSON failed");
        return -1; 
    }
    return pjson;
}

static class_info_entry_t Config_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_FUNC_POINTER, "", "load", load, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_FUNC_POINTER, "", "parsejson", parsejson, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Config", Config_class_info);