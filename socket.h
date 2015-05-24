#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "util.h"

class SocketInterface:public Thread
{
    char _addr[256];
    int _port;
    EventQueue & _eq;
    bool _shutdown;
  public:
    SocketInterface (const char* addr, int port, EventQueue & eq);
    void shutdown();
    virtual void run ();
};

#endif

