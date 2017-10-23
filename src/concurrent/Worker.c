/**
 * @file concurrent.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-09-24
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
#include <libobject/concurrent/worker.h>
#include <libobject/concurrent/producer.h>

static int __construct(Worker *worker,char *init_str)
{
    allocator_t *allocator = worker->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"worker construct, worker addr:%p",worker);

    return 0;
}

static int __deconstrcut(Worker *worker)
{
    dbg_str(EV_DETAIL,"worker deconstruct,worker addr:%p",worker);

    return 0;
}

static int __set(Worker *worker, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        worker->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        worker->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        worker->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        worker->deconstruct = value;
    }
    else if (strcmp(attrib, "assign") == 0) {
        worker->assign = value;
    } else if (strcmp(attrib, "enroll") == 0) {
        worker->enroll = value;
    } else if (strcmp(attrib, "resign") == 0) {
        worker->resign = value;
    } 
    else {
        dbg_str(EV_DETAIL,"worker set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Worker *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"worker get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int  __assign(Worker *worker, int fd, int ev_events,
                     struct timeval ev_tv, void *ev_callback,
                     void *ev_arg, void *work_callback)
{
    event_t *event = &worker->event;

    event->ev_fd          = fd;
    event->ev_events      = ev_events;
    event->ev_timeout     = ev_tv;
    event->ev_callback    = ev_callback;
    event->ev_arg         = ev_arg;
    worker->work_callback = work_callback;
}

static int __enroll(Worker *worker, void *producer)
{
}

static int __resign(Worker *worker, void *producer)
{
}

static class_info_entry_t worker_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Obj", "obj", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "assign", __assign, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "enroll", __enroll, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "resign", __resign, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_END},
};
REGISTER_CLASS("Worker", worker_class_info);

static void
test_timeout_cb(int fd, short event, void *arg)
{
    struct timeval newtime, difference;
    double elapsed;
    static struct timeval lasttime;

    gettimeofday(&newtime, NULL);
    timeval_sub(&newtime, &lasttime, &difference);

    elapsed  = difference.tv_sec + (difference.tv_usec / 1.0e6);
    lasttime = newtime;

    dbg_str(DBG_SUC,"timeout_cb called at %d: %.3f seconds elapsed.",
            (int)newtime.tv_sec, elapsed);
}

void test_obj_worker()
{
    Worker *worker;
    Producer *producer;
    allocator_t *allocator = allocator_get_default_alloc();
    struct timeval ev_tv;

    ev_tv.tv_sec  = 2;
    ev_tv.tv_usec = 0;

    producer = OBJECT_NEW(allocator, Producer, NULL);
    producer->start(producer);

    sleep(1);
    worker   = OBJECT_NEW(allocator, Worker, NULL);
    dbg_str(DBG_DETAIL,"run at here");
    worker->assign(worker, -1, EV_READ | EV_PERSIST,
                   ev_tv, test_timeout_cb, worker, NULL);

    producer->add_worker(producer, worker);

    pause();
    object_destroy(worker);
    object_destroy(producer);
}
