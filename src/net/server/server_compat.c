/**
 * @file Inet_Tcp_Server.c
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/config/config.h>
#include <libobject/utils/timeval/timeval.h>
#include <libobject/net/server/inet_tcp_server.h>

#define SERVER_TYPE_INET_TCP "inet_tcp_server_type"
#define SERVER_TYPE_UNIX_TCP "tcp_userver_type"
void *server(allocator_t *allocator, 
             char *type,
             char *host,
             char *service,
             void (*process_task_cb)(void *arg),
             void *opaque)
{
    Server *server;

    if(!strcmp(type,SERVER_TYPE_INET_TCP)){
        server = OBJECT_NEW(allocator, Inet_Tcp_Server, NULL);
        server->bind(server, host, service); 
        if (process_task_cb != NULL)
            server->trustee(server, (void *)process_task_cb, opaque);
    } else {
        dbg_str(DBG_WARNNING,"server type error");
        return NULL;
    }

    return (void *)server;
}

int server_destroy(void *server)
{
    Server *s = (Server *)server;
    return object_destroy(server);
}

static void test_work_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_SUC,"%s", t->buf);
    dbg_str(DBG_SUC,"task opaque=%p", t->opaque);
}

void test_obj_server()
{
    Server *s;
    allocator_t *allocator = allocator_get_default_alloc();

    s = (Server *)server(allocator, SERVER_TYPE_INET_TCP, 
                         "127.0.0.1", "11011", test_work_callback, allocator);

    dbg_str(DBG_SUC,"opaque=%p", allocator);
    pause();

    object_destroy(s);
}