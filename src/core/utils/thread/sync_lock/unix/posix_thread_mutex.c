/*
 * =====================================================================================
 *
 *       Filename:  linux_user_mode_mutex.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/24/2015 03:32:06 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <libobject/user_mode.h>
#include <libobject/core/utils/thread/sync_lock.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))

#include <pthread.h>

int posix_thread_mutex_init(struct sync_lock_s *slock)
{
    return pthread_mutex_init(&slock->lock.mutex, NULL);
}
int posix_thread_mutex_lock(struct sync_lock_s *slock, void *arg)
{
    return pthread_mutex_lock(&slock->lock.mutex);
}
int posix_thread_mutex_trylock(struct sync_lock_s *slock, void *arg)
{
    return pthread_mutex_trylock(&slock->lock.mutex);
}
int posix_thread_mutex_unlock(struct sync_lock_s *slock)
{
    return pthread_mutex_unlock(&slock->lock.mutex);
}
int posix_thread_mutex_lock_destroy(struct sync_lock_s *slock)
{
    return pthread_mutex_destroy(&slock->lock.mutex);
}
int  linux_user_mode_pthread_mutex_register(){
    sync_lock_module_t slm = {
        .name = "pthread_mutex", 
        .sync_lock_type = PTHREAD_MUTEX_LOCK, 
        .sl_ops = {
            .sync_lock_init    = posix_thread_mutex_init, 
            .sync_lock         = posix_thread_mutex_lock, 
            .sync_trylock      = posix_thread_mutex_trylock, 
            .sync_unlock       = posix_thread_mutex_unlock, 
            .sync_lock_destroy = posix_thread_mutex_lock_destroy, 
        }, 
    };
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register sync lock pthread_mutex module\n", 
                 REGISTRY_CTOR_PRIORITY_SYNC_LOCK_REGISTER_MODULES);
    memcpy(&sync_lock_modules[PTHREAD_MUTEX_LOCK], &slm, sizeof(sync_lock_module_t));
    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_SYNC_LOCK_REGISTER_MODULES, 
                   linux_user_mode_pthread_mutex_register);


#endif
