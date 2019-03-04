/**
 * @file ConfigFile.c
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/conf/configfile.h>
#include <libobject/core/linked_list.h>


static int __construct(ConfigFile *config,char *init_str)
{
    allocator_t *allocator = config->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(DBG_DETAIL,"config construct, config addr:%p",config);
    config->map      = OBJECT_NEW(allocator,RBTree_Map,NULL);
    config->map->set_cmp_func(config->map, string_key_cmp_func);
    return 0;
}

static int __deconstrcut(ConfigFile *config)
{
    dbg_str(DBG_DETAIL,"config deconstruct,config addr:%p",config);

    if (config->map) {
        if (!config->map->is_empty(config->map)) {
            config->map->clear_mem(config->map);
        }
        object_destroy(config->map);
    }
    return 0;
}

static int __set(ConfigFile *config, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        config->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        config->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        config->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        config->deconstruct = value;
    } else if (strcmp(attrib, "load_file") == 0) {
        config->load_file = value;
    } else if (strcmp(attrib, "get_value") == 0) {
        config->get_value = value;
    } else if (strcmp(attrib, "parse_line_internal") == 0) {
        config->parse_line_internal = value;
    } 
    else {
        dbg_str(DBG_DETAIL,"config set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(ConfigFile *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(DBG_WARNNING,"config get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __load_file(ConfigFile * config,const char * name)
{   
    char * p   = NULL;
    int  len   = 0;
    int  ret   = 0;
    char buf[1024];
    memset(buf,0,1024);

    if (name == NULL) {
        return -1;
    }

    FILE * fp = fopen(name,"r");
    if (NULL == fp) {
        dbg_str(DBG_ERROR,"can't find configfile %s",name);
        goto error;
    }

    while (1) {

        memset(buf,0,1024);
        p = fgets(buf,1024,fp);
        if (p == NULL) {
            if (feof(fp)) {
                break;
            } else {
                dbg_str(DBG_ERROR,"Fgets exception happen");
                goto error;
            }
        }

        len = strlen(buf);
        if (buf[len - 1] == '\n') { //读取完整一行
            buf[len - 1] = '\0';
            ret = config->parse_line_internal(config,buf);
            if (ret < 0) {
                dbg_str(DBG_ERROR,"parse config failed");
                goto error;
            } 
        } else { // 超出1024个字符 ，不做处理直接跳出
            dbg_str(DBG_ERROR,"config current line text over 1024");
            goto error;
        }

    }
    
    if (fp)
        fclose(fp);
    return 0;
error:
    if (fp)
        fclose(fp);
    return -1;
}

static int  __is_comment_line(char *buf)
{
    return buf[0] == '#' ? 1 : 0;
}

static char * __skip_lspace(char *buf)
{   
    int i    = 0;
    int len  = strlen(buf);
    char * p = NULL;

    for (i = 0 ; i < len ;i++) {
        if (buf[i] == ' ' || buf[i] == '=')
            continue;
        else {
            p = buf + i;
            break;
        }
    }
    return p;
}

static char * __skip_rspace(char *buf)
{   
    int i    = 0;
    int len  = strlen(buf);
    char * p = buf;

    for (i = len - 1 ; i >= 0 ;i--) {
        if (buf[i] == ' ' || buf[i] == '=') {
            buf[i] = '\0';
        }
        else {
            break;
        }
    }
    return p;
}

static int __is_null_line(char *buf)
{
    int len = strlen(buf);
    return len > 0? 0:1;
}

static char * __trim_space_line(char *buf)
{
    char *p  = buf;
    p = __skip_lspace(p);
    p = __skip_rspace(p);
    return p;
}

static int __parse_line_internal(ConfigFile * config,void *buf)
{
    int ret  = -1,len = 0 ,pos = 0,i = 0;
    char * p = NULL,*q = NULL;
    char *key ,*value  =  NULL;
    Map * map  = config->map;
    p = buf;

    allocator_t * allocator = config->obj.allocator;
    
    ret = __is_null_line(p);
    if (ret) {  return 1;}

    p   = __trim_space_line(p); 
    ret = __is_comment_line(p);
    if (ret) { return 1;} 

    len = strlen(p);
    q = strchr(p,'=');

    if (q == NULL) {
        return -1;
    }

    pos = q - p;
    p[pos] = '\0';

    q = p;
    p = p + pos + 1;

    q = __trim_space_line(q);
    p = __trim_space_line(p);
    len = strlen(q);

    key = allocator_mem_alloc(allocator,sizeof(char)*len + 1);

    if (key == NULL) {
        dbg_str(DBG_ERROR,"create key alloc failed")
        goto error;
    }

    memcpy(key,q,strlen(q));
    key[len] = '\0';
    len = strlen(p);

    dbg_str(DBG_ERROR,"current key %s value %s len:%d",q,p,len);

    value = allocator_mem_alloc(allocator,sizeof(char)*len + 1);
    
    if (value == NULL) {
        dbg_str(DBG_ERROR,"create key alloc failed")
        goto error;
    }

    memcpy(value,p,len);
    value[len] = '\0';
    map->add(map,key,value);

    dbg_str(DBG_ERROR,"key:%s value:%s len:%d",key,value,len); 
    return 0;
error:
    if (key) {
        allocator_mem_free(allocator,key);
    }
    if (value) {
        allocator_mem_free(allocator,value);
    }
    return -1;
}

static int __get_value(ConfigFile * config,const char * key,void **value)
{   
    int ret = 0;
    Map * map = config->map;

    if (map->is_empty(map)) {
        dbg_str(DBG_ERROR,"config file is null");
        return -1;
    }
    
    ret = map->search(map,(void *)key,value);
    if (ret < 0) {
        dbg_str(DBG_ERROR,"key:%s is null",key);
    }

    return ret;
}

static class_info_entry_t concurent_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_FUNC_POINTER,"","load_file",__load_file,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_FUNC_POINTER,"","get_value",__get_value,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_FUNC_POINTER,"","parse_line_internal",__parse_line_internal,sizeof(void *)},
    [8 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("ConfigFile",concurent_class_info);

static int test_config_file(TEST_ENTRY *entry)
{
    allocator_t * allocator = allocator_get_default_alloc();
    void * key;
    void *value;
    int ret = 0;

    ConfigFile * config = OBJECT_NEW(allocator,ConfigFile,NULL);
    
    config->load_file(config,"goya.conf");

    ret = config->get_value(config,"ip",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }
    ret = config->get_value(config,"password1",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"name",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"port",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"xxxx",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"listen",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"xxxxxxxxx",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }

    ret = config->get_value(config,"password",(void**)&value);
    if (ret >= 0) {
        dbg_str(DBG_SUC,"key:ip value:%s",(char *)value);
    }


    object_destroy(config);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_config_file);