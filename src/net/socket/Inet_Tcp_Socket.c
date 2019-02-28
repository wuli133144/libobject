/**
 * @file Socket_Udp.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-28
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
#include <libobject/event/event_base.h>
#include <libobject/net/socket/inet_tcp_socket.h>
#include <libobject/core/thread.h>

static int __construct(Inet_Tcp_Socket *sk, char *init_str)
{
    int skfd;

    dbg_str(NET_DETAIL, "socket construct, socket addr:%p", sk);
    if (sk->parent.fd > 0) {
        sk->close(sk);
        sk->parent.fd = -1;
    }

    if ((skfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("udp socket");
        return -1;
    }

    sk->parent.fd = skfd;
    sk->timeout   = 3;
    return 0;
}

static int __deconstrcut(Inet_Tcp_Socket *socket)
{
    dbg_str(NET_DETAIL, "socket deconstruct, socket addr:%p", socket);

    close(socket->parent.fd);

    return 0;
}

static int __set(Inet_Tcp_Socket *socket, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        socket->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        socket->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        socket->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        socket->deconstruct = value;
    } else if (strcmp(attrib, "bind") == 0) {
        socket->bind = value;
        dbg_str(DBG_DETAIL, "set bind, addr:%p", socket->bind);
    } else if (strcmp(attrib, "listen") == 0) {
        socket->listen = value;
    } else if (strcmp(attrib, "accept") == 0) {
        socket->accept = value;
    } else if (strcmp(attrib, "accept_fd") == 0) {
        socket->accept_fd = value;
    } else if (strcmp(attrib, "connect") == 0) {
        socket->connect = value;
    } else if (strcmp(attrib, "write") == 0) {
        socket->write = value;
    } else if (strcmp(attrib, "sendto") == 0) {
        socket->sendto = value;
    } else if (strcmp(attrib, "sendmsg") == 0) {
        socket->sendmsg = value;
    } else if (strcmp(attrib, "read") == 0) {
        socket->read = value;
    } else if (strcmp(attrib, "recv") == 0) {
        socket->recv = value;
    } else if (strcmp(attrib, "recvfrom") == 0) {
        socket->recvfrom = value;
    } else if (strcmp(attrib, "setblock") == 0) {
        socket->setblock = value;
    } else if (strcmp(attrib, "close") == 0) {
        socket->close = value;
    } else if (strcmp(attrib, "shutdown") == 0) {
        socket->shutdown = value;
    } else if (strcmp(attrib, "setnoclosewait") == 0) {
        socket->setnoclosewait = value;
    } else if (strcmp(attrib, "setclosewait") == 0) {
        socket->setclosewait = value;
    } else if (strcmp(attrib, "recvmsg") == 0) {
        socket->recvmsg = value;
    }  else if (strcmp(attrib, "send") == 0) {
        socket->send = value;
    } else if (strcmp(attrib, "get_timeout") == 0) {
        socket->get_timeout = value;
    } else if (strcmp(attrib, "set_timeout") == 0) {
        socket->set_timeout = value;
    } else if (strcmp(attrib, "get_socketfd") == 0) {
        socket->get_socketfd = value;
    } else if (strcmp(attrib, "setnonblocking") == 0) {
        socket->setnonblocking = value;
    } else if (strcmp(attrib, "setsockopt") == 0) {
        socket->setsockopt = value;
    } else if (strcmp(attrib, "getsockopt") == 0) {
        socket->getsockopt = value;
    } else if (strcmp(attrib, "setsendbuffer") == 0) {
        socket->setsendbuffer = value;
    } else if (strcmp(attrib, "setrecvbuffer") == 0) {
        socket->setrecvbuffer = value;
    } else if (strcmp(attrib, "send_async") == 0) {
        socket->send_async = value;
    } else if (strcmp(attrib, "recv_async") == 0) {
        socket->recv_async = value;
    } 
    else {
        dbg_str(NET_DETAIL, "socket set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Inet_Tcp_Socket *socket, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(NET_WARNNING, "socket get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static Socket * __accept(Inet_Tcp_Socket*socket, 
                         char *remote_host, char *remote_service)
{
    struct sockaddr_storage cliaddr;
    socklen_t len;
    Inet_Tcp_Socket * so = ( Inet_Tcp_Socket *)socket;
    allocator_t *allocator = so->parent.obj.allocator;
    int connfd;
    Socket *ret = NULL;

    while((connfd = accept(so->parent.fd, (struct sockaddr *)&cliaddr, &len)) < 0 && errno == EINTR);

    if (connfd > 0) {
        ret = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);
        ret->fd = connfd;
    } 

    return ret;
}

static int __accept_fd(Inet_Tcp_Socket*socket, 
                       char *remote_host, char *remote_service)
{
    struct sockaddr_storage cliaddr;
    socklen_t len;
    Inet_Tcp_Socket * so = (Inet_Tcp_Socket *)socket;
    allocator_t *allocator = so->parent.obj.allocator;

    return accept(so->parent.fd, (struct sockaddr *)&cliaddr, &len);
}

static void __setblock(Inet_Tcp_Socket *socket,int ihow)
{
    dbg_str(DBG_SUC,"Inet_Tcp_Socket setblock success");
    return ;
}

static int __inet_tcp_connect_internal(Inet_Tcp_Socket * socket, char *host, char *service)
{
    struct addrinfo  *addr, *addrsave, hint;
    int skfd, ret;
    char *h, *s;

    bzero(&hint, sizeof(hint));
    hint.ai_family   = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    if (host == NULL && service == NULL) {
        // h = socket->remote_host;
        // s = socket->remote_service;
    } else {
        h = host;
        s = service;
    }

    if ((ret = getaddrinfo(h, s, &hint, &addr)) != 0){
        dbg_str(DBG_ERROR, "getaddrinfo error: %s", gai_strerror(ret));
        return -1;
    }
    addrsave = addr;

    if (addr != NULL) {
        dbg_str(NET_DETAIL, "ai_family=%d type=%d", addr->ai_family, addr->ai_socktype);
    } else {
        dbg_str(DBG_ERROR, "getaddrinfo err");
        return -1;
    }                      

    do {
        if ((ret = connect(socket->parent.fd, addr->ai_addr, addr->ai_addrlen)) == 0)
            break;
    } while ((addr = addr->ai_next) != NULL);


    freeaddrinfo(addrsave);

    return ret;
}

static int __connect_async(Inet_Tcp_Socket * so, char *host, char *service)
{   
    int ret = -1,ret_select = -1;
    int error = 0;
    socklen_t len = sizeof(error);
    
    //----设置非阻塞----
    so->setblock(so,0);
    ret = __inet_tcp_connect_internal(so,host,service);
    #if 1

    if (ret == 0) {
        dbg_str(DBG_SUC,"connect success!");
        so->setblock(so,1);
        return ret;
    } else if (ret < 0  && errno == EINPROGRESS) {
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(so->parent.fd,&wset);
        struct timeval te_tv;
        te_tv.tv_sec   = so->timeout;
        te_tv.tv_usec  = 0;

        ret_select = select(so->parent.fd+1,NULL,&wset,NULL,&te_tv);
        switch (ret_select)
        {
            case -1:
                /* code */
                so->close(so);
                break;
            case 0: 
                so->close(so);
                break;
            default:
               
                if (getsockopt(so->parent.fd,SOL_SOCKET,SO_ERROR,&error,&len) < 0) {
                   so->close(so);
                   break;
                }

                if (error != 0) {
                   so->close(so);
                   break;
                }
                //设置阻塞
                so->setblock(so,1);
                dbg_str(DBG_SUC,"connect success!");
                return 0;
        }
    } else {
         so->close(so);
    }
    #endif 

    if (ret != 0 ) {
        dbg_str(DBG_ERROR,"connect failed!");
    }

    return -1;
}

static  socket_status_t __recv_async(Inet_Tcp_Socket * socket, void *buf, size_t *len, int flags)
{
    int ret;
    ssize_t recvlen;
    fd_set read_set;
    struct timeval tm_tv;
    int fd = socket->get_socketfd(socket);
    tm_tv.tv_sec  = socket->timeout;
    tm_tv.tv_usec = 0;

    FD_ZERO(&read_set);
    FD_SET(fd,&read_set);

    ret = select(fd+1,&read_set,NULL,NULL,&tm_tv);

    switch (ret)
    {
        case -1:
            /* code */
            socket->close(socket);
            return NET_SOCKET_SELECT;
        case 0:
            socket->close(socket);
            return NET_SOCKET_TIMEOUT;
        default:

            if (FD_ISSET(fd,&read_set)) {
                recvlen = recv(fd,buf,*len,flags);
                if (recvlen < 0) {
                    socket->close(socket);
                    return NET_SOCKET_RECV;
                }  else if(recvlen == 0) {
                    socket->close(socket);
                    return  NET_SOCKET_CLOSE;
                }

                *len =  recvlen;

                return NET_SOCKET_SUCCESS;
            }
    }
    
}

static  socket_status_t __send_async(Inet_Tcp_Socket * socket, void *buf, size_t *len, int flags)
{
    int ret = 0,fd = -1,sendlen = -1;
    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(socket->parent.fd,&wset);
    struct timeval tv_tm;
    tv_tm.tv_sec  = socket->timeout;
    tv_tm.tv_usec = 0; 
    fd = socket->parent.fd;
    
    ret = select(fd+1,NULL,&wset,NULL,&tv_tm);

    switch(ret) {
        case -1:
            socket->close(socket);
            return NET_SOCKET_SELECT;
        case 0:
            socket->close(socket);
            return NET_SOCKET_TIMEOUT;
        default:

            if (FD_ISSET(fd,&wset)) {
                sendlen = send(fd,buf,*len,flags);
                if (sendlen < 0) {
                    socket->close(socket);
                    return NET_SOCKET_SEND;
                } else if (sendlen == 0) {
                    socket->close(socket);
                    return NET_SOCKET_CLOSE;
                }

                *len = sendlen;

                return NET_SOCKET_SUCCESS;
            }
    }   

}

static void __set_timeout(Inet_Tcp_Socket *socket,int timeout)
{
    socket->timeout = timeout;
}

static int __get_timeout(Inet_Tcp_Socket *socket)
{
    return socket->timeout;
}

static int __get_socketfd(Inet_Tcp_Socket *socket)
{
    return socket->parent.fd;
}

static int __setrecvbuffer(Inet_Tcp_Socket *socket,int nRecvBuf)
{
    int ret  = -1;
    ret = setsockopt(socket->parent.fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
    if (ret < 0) {
        dbg_str(DBG_ERROR,"set socket recv buffersize failed! close socket")
        socket->close(socket);
        return ret;
    } 

    return ret;
}

static int __setsendbuffer(Inet_Tcp_Socket *socket,int nSendBuf)
{
    int ret  = -1;
    ret = setsockopt(socket->parent.fd,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));
    if (ret < 0) {
        dbg_str(DBG_ERROR,"set socket send buffersize failed! close socket")
        socket->close(socket);
        return ret;
    } 

    return ret;  
}

static class_info_entry_t inet_tcp_socket_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,     "Socket", "parent", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "",  "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "",  "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "",  "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "",  "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "bind", NULL, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "listen", NULL, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "accept", __accept, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "accept_fd", __accept_fd, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_IFUNC_POINTER, "", "connect", NULL, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_IFUNC_POINTER, "", "write", NULL, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_IFUNC_POINTER, "", "sendto", NULL, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_IFUNC_POINTER, "", "sendmsg", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_IFUNC_POINTER, "", "read", NULL, sizeof(void *)}, 
    [14] = {ENTRY_TYPE_IFUNC_POINTER, "", "recv", NULL, sizeof(void *)}, 
    [15] = {ENTRY_TYPE_IFUNC_POINTER, "", "recvfrom", NULL, sizeof(void *)}, 
    [16] = {ENTRY_TYPE_IFUNC_POINTER, "", "recvmsg", NULL, sizeof(void *)},
    [17] = {ENTRY_TYPE_IFUNC_POINTER, "", "close", NULL, sizeof(void *)},
    [18] = {ENTRY_TYPE_IFUNC_POINTER, "", "setblock", NULL, sizeof(void *)},
    [19] = {ENTRY_TYPE_IFUNC_POINTER, "", "shutdown", NULL, sizeof(void *)},
    [20] = {ENTRY_TYPE_IFUNC_POINTER, "", "setnoclosewait", NULL, sizeof(void *)},
    [21] = {ENTRY_TYPE_IFUNC_POINTER, "", "setclosewait", NULL, sizeof(void *)},
    [22] = {ENTRY_TYPE_IFUNC_POINTER, "", "setclosewaitdefault", NULL, sizeof(void *)},
    [23] = {ENTRY_TYPE_FUNC_POINTER, "",  "set_timeout", __set_timeout, sizeof(void *)},
    [24] = {ENTRY_TYPE_FUNC_POINTER, "",  "get_timeout", __get_timeout, sizeof(void *)},
    [25] = {ENTRY_TYPE_IFUNC_POINTER, "", "send", NULL, sizeof(void *)},
    [26] = {ENTRY_TYPE_VFUNC_POINTER, "", "get_socketfd", __get_socketfd, sizeof(void *)},
    [27] = {ENTRY_TYPE_IFUNC_POINTER, "", "getsockopt", NULL, sizeof(void *)},
    [28] = {ENTRY_TYPE_IFUNC_POINTER, "", "setsockopt", NULL, sizeof(void *)},
    [29] = {ENTRY_TYPE_IFUNC_POINTER, "", "setnoblocking", NULL, sizeof(void *)},
    [30] = {ENTRY_TYPE_VFUNC_POINTER, "", "setsendbuffer", __setsendbuffer, sizeof(void *)},
    [31] = {ENTRY_TYPE_VFUNC_POINTER, "", "setrecvbuffer", __setrecvbuffer, sizeof(void *)},
    [32] = {ENTRY_TYPE_VFUNC_POINTER, "", "connect_async", __connect_async, sizeof(void *)},
    [33] = {ENTRY_TYPE_VFUNC_POINTER, "", "send_async", __send_async, sizeof(void *)},
    [34] = {ENTRY_TYPE_VFUNC_POINTER, "", "recv_async", __recv_async, sizeof(void *)},
    [35] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Inet_Tcp_Socket", inet_tcp_socket_class_info);

int test_inet_tcp_socket_send(TEST_ENTRY *entry)
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "GET / HTTP/1.0 \r\n\r\n";
    int ret = 0;
    ssize_t len = -1;
    char buf[1024]={0};
    strncpy(buf,test_str,strlen(test_str));

    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    socket->set_timeout(socket,2);

    ret = socket->connect(socket, "111.13.100.92", "8080");

    if (ret == 0) {
        dbg_str(DBG_SUC,"connect success");
    } else {
        dbg_str(DBG_ERROR,"connect failed");
    }

    dbg_str(DBG_SUC,"socket fd %d",socket->get_socketfd(socket));
    
    ret = socket->send(socket,buf,len,0);
    // dbg_str(DBG_SUC,"socket send %d",ret);
    //socket->write(socket, test_str, strlen(test_str));
    bzero(buf,1024);
    len = socket->recv(socket,buf,1024,0);
    dbg_str(DBG_SUC,"connect success %d %s",len,buf);
    pause();
    //while(1) sleep(1);

    object_destroy(socket);
    return 1;
}

void test_inet_tcp_socket_recv(TEST_ENTRY *entry)
{
    Socket *socket, *new;
    char buf[1024] = {0};
    allocator_t *allocator = allocator_get_default_alloc();

    /*
     *dbg_str(NET_DETAIL, "run at here");
     */
    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    dbg_str(DBG_DETAIL, "sizeof socket=%d", sizeof(Socket));
    socket->bind(socket, "127.0.0.1", "11011"); 
    socket->listen(socket, 1024);
    new = socket->accept(socket, NULL, NULL);

    new->read(new, buf, 1024);
    dbg_str(NET_SUC, "recv : %s", buf);

    sleep(10);

    object_destroy(new);
    object_destroy(socket);
}

int test_inet_tcp_socket_async(TEST_ENTRY * entry)
{
    Socket *socket;
    allocator_t *allocator = allocator_get_default_alloc();

    char *test_str = "GET / HTTP/1.0 \r\n\r\n";
    int ret = 0;
    ssize_t len = -1;
    char buf[1024]={0};
    strncpy(buf,test_str,strlen(test_str));

    socket = OBJECT_NEW(allocator, Inet_Tcp_Socket, NULL);

    socket->set_timeout(socket,2);

    ret = socket->connect_async(socket, "111.13.100.92", "8080");

    if (ret == 0) {
        dbg_str(DBG_SUC,"connect success");
    } else {
        dbg_str(DBG_ERROR,"connect failed");
    }

    dbg_str(DBG_SUC,"socket fd %d",socket->get_socketfd(socket));
    
    ret = socket->send_async(socket,buf,len,0);
    // dbg_str(DBG_SUC,"socket send %d",ret);
    //socket->write(socket, test_str, strlen(test_str));
    bzero(buf,1024);
    len = socket->recv(socket,buf,1024,0);
    dbg_str(DBG_SUC,"connect success %d %s",len,buf);
    pause();
    //while(1) sleep(1);

    object_destroy(socket);
    return 1;
}

REGISTER_STANDALONE_TEST_FUNC(test_inet_tcp_socket_send);
REGISTER_STANDALONE_TEST_FUNC(test_inet_tcp_socket_async);