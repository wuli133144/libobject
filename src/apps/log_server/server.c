#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/client/client.h>
#include <libobject/net/client/inet_udp_client.h>
#include <libobject/net/client/inet_tcp_client.h>

static int test_work_callback(void *task)
{
    net_task_t *t = (net_task_t *)task;
    dbg_str(DBG_DETAIL,"%s", t->buf);
}

void log_server()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Client *c = NULL;

    dbg_str(DBG_DETAIL,"test_obj_client_recv");

    c = client(allocator,
               CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1",//char *host,
               (char *)"8080",//char *client_port,
               test_work_callback,
               NULL);
    pause();
    object_destroy(c);
}
