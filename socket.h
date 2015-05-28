#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "util.h"
#include <sys/socket.h>
class SocketInterface:public Thread
{
    char _addr[256];
    int _port;
    EventQueue & _eq;
    bool _shutdown;

    int _sock1;

    struct sockaddr _peer_addr;
    socklen_t _peer_addrlen;
  public:
    SocketInterface (const char* addr, int port, EventQueue & eq);
    void shutdown();
    void send(const char*);
    virtual void run ();
};

#endif

