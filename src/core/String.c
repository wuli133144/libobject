/**
 * @file string.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/string.h>
#include <libobject/core/vector.h>

static int string_buf_auto_modulate(String *string, int write_len)
{
    if (string->value_max_len == 0) {
        string->value_max_len  =  100;
        if (write_len > string->value_max_len) {
            string->value_max_len  =  write_len;
        }
        string->value = (char *)allocator_mem_alloc(string->obj.allocator, 
                                                    string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING, "string assign alloc error");
            return -1;
        }
    } else if ( string->value_max_len > string->value_len + 1 &&
               string->value_max_len < string->value_len + write_len + 1)
    {
        char *new_buf;

        string->value_max_len  =  2 * (string->value_len + write_len + 1);
        new_buf  =  (char *)allocator_mem_alloc(string->obj.allocator, 
                                              string->value_max_len);
        if (string->value == NULL) {
            dbg_str(OBJ_WARNNING, "string assign alloc error");
            return -1;
        }
        strncpy(new_buf, string->value, string->value_len);

        allocator_mem_free(string->obj.allocator, string->value);
        string->value  =  new_buf;
    }

    return 0;
}

static int __construct(String *string, char *init_str)
{
    dbg_str(OBJ_DETAIL, "string construct, string addr:%p", string);
    //maxsize 256
    string->value  =  (char *)allocator_mem_alloc(string->obj.allocator, 256);
    string->value_max_len  =  256;
    return 0;
}

static int __deconstrcut(String *string)
{
    dbg_str(OBJ_DETAIL, "string deconstruct, string addr:%p", string);
    if (string->value)
        allocator_mem_free(string->obj.allocator, string->value);

    return 0;
}

static int __set(String *string, char *attrib, void *value)
{

    if (strcmp(attrib, "set") == 0) {
        string->set  =  value;
    } else if (strcmp(attrib, "get") == 0) {
        string->get  =  value;
    } else if (strcmp(attrib, "construct") == 0) {
        string->construct  =  value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        string->deconstruct  =  value;
    } else if (strcmp(attrib, "pre_alloc") == 0) {
        string->pre_alloc  =  value;
    } else if (strcmp(attrib, "assign") == 0) {
        string->assign  =  value;
    } else if (strcmp(attrib, "append_char") == 0) {
        string->append_char  =  value;
    } else if (strcmp(attrib, "replace_char") == 0) {
        string->replace_char  =  value;
    }else if(strcmp(attrib, "toupper") == 0){
        string->toupper = value;
    }else if(strcmp(attrib, "toupper_impact") == 0){
        string->toupper_impact = value;
    }else if(strcmp(attrib, "lower") == 0){
        string->tolower = value;
    }else if(strcmp(attrib, "lower_impact") == 0){
        string->tolower_impact = value;
    }else if (strcmp(attrib, "at") == 0) {
        string->at  =  value;
    }else if(strcmp(attrib, "ltrim") == 0){
        string->ltrim = value; 
    }else if(strcmp(attrib, "rtrim") == 0){
        string->rtrim = value;
    }else if(strcmp(attrib, "trim") == 0){
        string->trim = value;
    }else if(strcmp(attrib, "split_string") == 0){
        string->split_string = value;
    }else if(strcmp(attrib, "find") == 0){
        string->find = value;
    }else if(strcmp(attrib, "substr") == 0){
        string->substr = value;
    }else if (strcmp(attrib, "name") == 0) {
        strncpy(string->name, value, strlen(value));
    } else {
        dbg_str(OBJ_DETAIL, "string set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(String *obj, char *attrib)
{
    if (strcmp(attrib, "name") == 0) {
        return obj->name;
    } else if (strcmp(attrib, "value") == 0) {
        return obj->value;
    } else {
        dbg_str(OBJ_WARNNING, "string get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

// Expand memory size
// return enough space
static String *__pre_alloc(String *string, uint32_t size)
{
    dbg_str(OBJ_DETAIL, "pre_alloc, size = %d", size);

    if (size < string->value_max_len) return string;
    else {
        allocator_mem_free(string->obj.allocator, string->value);
        string->value          =  (char *)allocator_mem_alloc(string->obj.allocator, size);
        string->value_max_len  =  size;
        memset(string->value, 0, size);
    }
    return string;
}

static String *__assign(String *string, char *s)
{
    int len  =  strlen(s);
    int ret;

    ret  =  string_buf_auto_modulate(string, len);
    if (ret < 0) return string;

    memset(string->value, 0, string->value_max_len);
    strncpy(string->value, s, len);
    string->value_len   =  len;
    string->value[len]  =  '\0';

    return string;
}

static String *__append_char(String *string, char c)
{
    int ret;

    ret  =  string_buf_auto_modulate(string, 1);
    if (ret < 0) {
        dbg_str(DBG_WARNNING, "string buf_auto_modulate have problem, please check");
        return string;
    }

    string->value[string->value_len]  =  c;
    string->value_len++;
    string->value[string->value_len]  =  '\0';

    return string;
}

static String *__replace_char(String *string, int index, char c)
{
    string->value[index]  =  c;

    return string;
}

static char __at(String *string, int index)
{
    return string->value[index];
}


static void __toupper_impact(String *string)
{
    int size = string->value_len;
    int i;
    for(i  =  0; i < size; i++){
        if(islower(string->value[i])){
            //toupper(string->value[i]);
            string->value[i]+= 'A'-'a';
        }
    }       
}

static void  __toupper_(String *string, String *str)
{
    str->assign(str, string->value);
    __toupper_impact(str);  
}

static void __tolower_impact(String *string)
{
    int size = string->value_len;
    int i;
    for(i  =  0; i < size; i++){
        if(isupper(string->value[i])){
            //tolower(string->value[i]);
            string->value[i]+= 'a'-'A';
        }
     } 
}

static void __tolower_(String *string, String *str)
{
    str->assign(str, string->value);
    __tolower_impact(str);
}

static void __ltrim(String *string)
{
    int size = string->value_len;
    int i;
    for(i  =  0; i < size; i++){
        /* code */
        if(isspace(string->value[i])){
            string->value++;
        }else{
            break;
        }   
       
    }
}

static void __rtrim(String *string)
{
    int size = string->value_len;
    int i;
    for(i = size-1; i >= 0; i--){
        /* code */
        if(isspace(string->value[i])){
            string->value[i] = '\0';
        }else{
            break;
        }   
    }
}

static void __trim(String *string)
{ 
    if(NULL != string){
        __ltrim(string);
        __rtrim(string);
    }
}

static int __find(String *string, String *substr, int pos)
{   
    int len1  =  string->value_len;
    int len2  =  substr->value_len;
    char *p   =  NULL;
    if(NULL == string || NULL == substr || pos < 0 || len1 < len2)
    {
        return -1;//exception happened
    }
    //a simple method
    p = strstr(string->value+pos, substr->value);
    if(NULL !=  p)
    {
        return p-(string->value);
    }
    return -1;
}

static String *__substr(String  *string, int pos, int len)
{
    int size = string->value_len;
    int i;
    
    String *str;
    //allocator_t *allocator  =  allocator_get_default_alloc();
    str = OBJECT_NEW(string->obj.allocator, String, NULL);
    assert(pos <=  size);
    for( i  = pos;i < size && len ;i++){
           str->append_char(str, string->value[i]);
           len--;
    }
    return str;
}

static void __split_string(String *string, String *separator, Vector *vector)
{
    int start_pos = 0, pos;
    String *pstr = NULL;
    int i = 0;
    
    if(NULL == string|| NULL == separator) {
        vector->add_back(vector, (void *)string);
        return;
    }
    
    while(1)
    {
        pos = __find(string, separator, start_pos);
        if(pos == start_pos){
            start_pos+= separator->value_len;
            continue;
        }
        if(pos < 0){   
             pstr = __substr(string, start_pos, string->value_len); 
             vector->add_at(vector, i, pstr);       
             break;
        }else{
             pstr = __substr(string, start_pos, pos-start_pos);
             vector->add_at(vector, i, pstr);
             start_pos = pos+separator->value_len;
        }
        i++;     
   }
}



static class_info_entry_t string_class_info[]  =  {
    [0 ]  =  {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "pre_alloc", __pre_alloc, sizeof(void *)}, 
    [6 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "assign", __assign, sizeof(void *)}, 
    [7 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "append_char", __append_char, sizeof(void *)}, 
    [8 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "replace_char", __replace_char, sizeof(void *)}, 
    [9 ]  =  {ENTRY_TYPE_FUNC_POINTER, "", "at", __at, sizeof(void *)}, 
    [10]  =  {ENTRY_TYPE_FUNC_POINTER, "", "toupper", __toupper_, sizeof(void *)}, 
    [11]  =  {ENTRY_TYPE_FUNC_POINTER, "", "toupper_impact", __toupper_impact, sizeof(void *)}, 
    [12]  =  {ENTRY_TYPE_FUNC_POINTER, "", "tolower_impact", __tolower_impact, sizeof(void *)}, 
    [13]  =  {ENTRY_TYPE_FUNC_POINTER, "", "tolower", __tolower_, sizeof(void *)}, 
    [14]  =  {ENTRY_TYPE_FUNC_POINTER, "", "ltrim", __ltrim, sizeof(void *)}, 
    [15]  =  {ENTRY_TYPE_FUNC_POINTER, "", "rtrim", __rtrim, sizeof(void *)}, 
    [16]  =  {ENTRY_TYPE_FUNC_POINTER, "", "trim", __trim, sizeof(void *)}, 
    [17]  =  {ENTRY_TYPE_FUNC_POINTER, "", "split_string", __split_string, sizeof(void *)}, 
    [18]  =  {ENTRY_TYPE_FUNC_POINTER, "", "substr", __substr, sizeof(void *)}, 
    [19]  =  {ENTRY_TYPE_FUNC_POINTER, "", "find", __find, sizeof(void *)}, 
    [20]  =  {ENTRY_TYPE_STRING, "char *", "name", NULL, 0}, 
    [21]  =  {ENTRY_TYPE_STRING, "char *", "value", NULL, 0}, 
    [22]  =  {ENTRY_TYPE_END}, 
};

REGISTER_CLASS("String", string_class_info);

#if  0
void test_obj_string()
{
    String *string;
    allocator_t *allocator  =  allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int alloc_count_be, alloc_count_end;

    dbg_str(DBG_DETAIL, "test_obj_string");
    alloc_count_be  =  allocator->alloc_count;

#if 0
    root  =  cjson_create_object();{
        cjson_add_item_to_object(root, "String", e  =  cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str  =  cjson_print(root);

    string  =  OBJECT_NEW(allocator, String, set_str);
    free(set_str);
#else

#define MAX_BUFFER_LEN 1024
    char config[MAX_BUFFER_LEN]  =  {0};

    object_config(config, MAX_BUFFER_LEN, "/String", OBJECT_STRING, "name", "alan") ;
    string   =  OBJECT_NEW(allocator, String, config);
#undef MAX_BUFFER_LEN
#endif

    string->pre_alloc(string, 1024);
    string->assign(string, "hello world!");
    string->append_char(string, 'a');
    string->append_char(string, 'b');

    object_dump(string, "String", buf, 2048);
    dbg_str(DBG_DETAIL, "String dump: %s", buf);
    
    memset(buf, 0, sizeof(buf));
    string->toupper(string);
    object_dump(string, "String", buf, 2048);
    dbg_str(DBG_DETAIL, "String dump: %s", buf);

    object_destroy(string);

    alloc_count_end  =  allocator->alloc_count;
    if (alloc_count_be !=  alloc_count_end) {
        dbg_str(DBG_WARNNING, "there's mem leak in test_obj_string test");
    }

}
#else 


 static void print_vector_data(int index, void *element)
{  
    if(element !=  NULL){
        printf(" index:%d value:%s  type_name:%s\n", index, ((String*)element)->value, ((String*)element)->obj.name);
    }
}

static void free_vector_elements(int index, void *element)
{
    if(element !=  NULL) { 
         object_destroy((String *)element);
        //printf(" %d %s \n", index, ((String*)element)->value);
        //free(element);
        element = NULL;
        //printf("release free memory index :%d\n", index);
    }
}


 int test_obj_string_split_string()
{    
   
   allocator_t *allocator2  =  allocator_get_default_alloc();
   //test find and split_string function
   int count = allocator2->alloc_count;
   String *str_find, *str_separator;
   str_find = OBJECT_NEW(allocator2, String, NULL);
  
   str_find->assign(str_find, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
   "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
   "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
   "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
   "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");  

   
   printf("str_find type_name:%s\n", str_find->obj.name);

   str_separator = OBJECT_NEW(allocator2, String, NULL);
   #if 1
   str_separator->assign(str_separator, "&");
   //str_separator->assign(str_separator, "//");
   #else 
   str_separator->assign(str_separator, "//");
   #endif 
   Vector *vc;
   vc = OBJECT_NEW(allocator2, Vector, NULL);
   vector_init(vc->vector, 10, 10);
   str_find->split_string(str_find, str_separator, vc);
   
   vc->for_each(vc, print_vector_data);
   vc->for_each(vc, free_vector_elements);

   object_destroy(str_find);
   object_destroy(str_separator);
   object_destroy(vc);
   //vector_destroy(vc->vector);
   int end = allocator2->alloc_count;
   printf("leak memory number %d end %d count %d ", end-count, end, count);
   return 1;

}

int test_string_find()
{
   allocator_t *allocator = allocator_get_default_alloc();
   String *string, *pstr;
   string                 = OBJECT_NEW(allocator, String, NULL);
   string->assign(string, "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");
   pstr                   = OBJECT_NEW(allocator, String, NULL);
   pstr->assign(pstr, "&");
   int pos = string->find(string, pstr, 0);
   printf("substr position: %d ", pos);
   return 1;
   
}

int test_string_substr()
{
   allocator_t *allocator = allocator_get_default_alloc();
   String *string, *pstr;
   string                 = OBJECT_NEW(allocator, String, NULL);
   string->assign(string, "&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25");
   pstr                   = OBJECT_NEW(allocator, String, NULL);
   pstr->assign(pstr, "&");
   pstr = string->substr(string, 3, 100);
   printf("substr %s\n ", pstr->value);
   return 1;   
}


 int test_obj_string()
{
    String *string;
    allocator_t *allocator  =  allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];
    int alloc_count_be, alloc_count_end;

    dbg_str(DBG_DETAIL, "test_obj_string");
    alloc_count_be  =  allocator->alloc_count;

#if 0
    root  =  cjson_create_object();{
        cjson_add_item_to_object(root, "String", e  =  cjson_create_object());{
            cjson_add_string_to_object(e, "name", "alan");
        }
    }

    set_str  =  cjson_print(root);

    string  =  OBJECT_NEW(allocator, String, set_str);
    free(set_str);
#else

#define MAX_BUFFER_LEN 1024
    char config[MAX_BUFFER_LEN]  =  {0};  

    object_config(config, MAX_BUFFER_LEN, "/String", OBJECT_STRING, "name", "alan") ;
    string   =  OBJECT_NEW(allocator, String, config);
#undef MAX_BUFFER_LEN
#endif

    string->pre_alloc(string, 1024);
    string->assign(string, "hello world!");
    string->append_char(string, 'a');
    string->append_char(string, 'b');

    object_dump(string, "String", buf, 2048);
    dbg_str(DBG_DETAIL, "String dump: %s", buf);
    
    memset(buf, 0, sizeof(buf));
    
    //string->toupper_impact(string);
    //object_dump(string, "String", buf, 2048);
    //dbg_str(DBG_DETAIL, "String dump: %s", buf);
    //printf("  result:%s\n", string->value);
    String *str;
    allocator_t *allocator1  =  allocator_get_default_alloc();
    str = OBJECT_NEW(allocator1, String, NULL);
    string->toupper(string, str);
    //str->ltrim(str);
    printf("orginal string %s  result:%s\n", string->value, str->value);
    
    //test trim
    String *str_trim;
    allocator_t *allocator2  =  allocator_get_default_alloc();

    alloc_count_be = allocator2->alloc_count;
    str_trim = OBJECT_NEW(allocator2, String, NULL);
    //str_trim->pre_alloc(str_trim, 1024);
    str_trim->assign(str_trim, "    fasdfasdfdas  ");
    //str_trim->rtrim(str_trim);
    printf("original_trim_result:%sxxxxxxxxxxxxx\n", str_trim->value);
    str_trim->ltrim(str_trim);
    printf("trim_result:%sxxxxxxxxxxxxx\n", str_trim->value);
    
   //test find and split_string function
   
   String *str_find, *str_separator;
   str_find = OBJECT_NEW(allocator2, String, NULL);
   //str_find();
   str_find->assign(str_find, "https://www.baidu.com/s?ie = utf-8&f = 3&rsv_bp = 1&rsv_idx = 1&tn = baidu&wd = "
   "ffmpeg%20hls%20%20%E6%A8%A1%E5%9D%97&oq = ffmpeg%2520hls%2520%25E5%2588%2587%25E7%2589%2587&rsv_pq = f57123dc00006105&"
   "rsv_t = 4a67K//PcOq6Y0swQnyeFtlQezzWiuwU1bS8vKp48Nn9joWPQd1BHAqFkqu9Y&rqlang = cn&rsv_enter = 1&inputT = 4580&"
   "rsv_sug3 = 170&rsv//_sug1 = 107&rsv_sug7 = 100&rsv_sug2 = 0&prefixsug = ffmpeg%2520hls%2520%2520%25E6%25A8%25A1%25"
   "E5%259D%2597&rsp = 0&rsv_sug4 = 5089");
   str_separator = OBJECT_NEW(allocator2, String, NULL);
   str_separator->assign(str_separator, "&");
   

   printf("parent_string:%s child_string:%s\n", str_find->value, str_separator->value);

   int pos = str_find->find(str_find, str_separator, 0);
   printf("find %s position %d", str_separator->value, pos);
    
    //test substr member function
    
    String *str_substring;
    str_substring = OBJECT_NEW(allocator2, String, NULL);
    str_substring->assign(str_substring, "f12asdfsd$afdsaf");
    String *pStr = str_substring->substr(str_substring, 4, 100);
    printf(" substring:%s\n", pStr->value);

    String *pStr1 = str_substring->substr(str_substring, 12, 100);
    printf("substring1:%s\n", pStr1->value);
    //test split_string
    Vector *vc;
    // allocator_t *allocator2 = allocator_get_default_alloc();
    vc = OBJECT_NEW(allocator2, Vector, NULL);
    str_find->split_string(str_find, str_separator, vc);
    //printf("vector size %d", vc->size);
    vc->for_each(vc, print_vector_data);
    //vc->for_each(vc, free_vector_elements);
    vc->free_vector(vc);
    object_destroy(vc);
    object_destroy(str_find);
    object_destroy(str_separator);
    object_destroy(str_substring);
    object_destroy(str_trim);
    object_destroy(str);
    object_destroy(string);
    object_destroy(pStr1);  
    alloc_count_end  =  allocator2->alloc_count;
    if (alloc_count_be !=  alloc_count_end) {
        dbg_str(DBG_WARNNING, "there's mem leak in test_obj_string test");
    }
    return 1;

}

#endif 

REGISTER_STANDALONE_TEST_FUNC(test_obj_string);
REGISTER_STANDALONE_TEST_FUNC(test_obj_string_split_string);
REGISTER_STANDALONE_TEST_FUNC(test_string_find);
REGISTER_STANDALONE_TEST_FUNC(test_string_substr);
