#include "socket.h"
#include "util.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

SocketInterface::SocketInterface (const char* addr, int port, EventQueue & eq):_eq (eq)
{
	strncpy(_addr, addr, 256-1);
	_port = port;
	_shutdown = false;
}

void
SocketInterface::shutdown()
{
    _shutdown = true; // TODO : lock
}

void
SocketInterface::send(const char* s)
{
   int sent = ::sendto(_sock1, s, strlen(s), 0, &_peer_addr, _peer_addrlen); 
   printf("Sent %s : %d, %d\n", s, sent, errno);
}

void
SocketInterface::run ()
{
    struct sockaddr_in addr1;
    fd_set fds, readfds;
    char buf[2048];
    int maxfd;
    int n;
    struct timeval tv;

    /* 受信ソケットを2つ作ります */
    _sock1 = socket (AF_INET, SOCK_DGRAM, 0);

    addr1.sin_family = AF_INET;

    addr1.sin_addr.s_addr = inet_addr (_addr);

    /* 2つの別々のポートで待つために別のポート番号をそれぞれ設定します */
    addr1.sin_port = htons (_port);

    /* 2つの別々のポートで待つようにbindします */
    bind (_sock1, (struct sockaddr *) &addr1, sizeof (addr1));

    /* fd_setの初期化します */
    FD_ZERO (&readfds);

    /* selectで待つ読み込みソケットとしてsock1を登録します */
    FD_SET (_sock1, &readfds);

    /* 10秒でタイムアウトするようにします */
    tv.tv_sec = 1;
    tv.tv_usec = 0;


    maxfd = _sock1;

    /* このサンプルでは、10秒間データを受信しないとループを抜けます */
    while (!_shutdown) { // TODO: lock
	/*
	   読み込み用fd_setの初期化
	   selectが毎回内容を上書きしてしまうので、毎回初期化します
	 */
	memcpy (&fds, &readfds, sizeof (fd_set));

	/* fdsに設定されたソケットが読み込み可能になるまで待ちます */
	n = select (maxfd + 1, &fds, NULL, NULL, &tv);

	/* タイムアウトの場合にselectは0を返します */
	if (n == 0) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        continue;
	}

	/* sock1に読み込み可能データがある場合 */
	if (FD_ISSET (_sock1, &fds)) {
	    /* sock1からデータを受信して表示します */
	    memset (buf, 0, sizeof (buf));
	    recvfrom (_sock1, buf, sizeof (buf), 0, &_peer_addr, &_peer_addrlen);
	    printf ("Receive : %s\n", buf);
            char dst[128];
            const char* next = token(buf, dst);
            if(strcmp(dst,"deploy") == 0){
                next = token(next, dst);
                _eq.push(new EventDeploy(dst));
            }else if(strcmp(dst,"gpio0") == 0){
                _eq.push(new Event(EVT_GPIO0_TRIGERRED));
	        }else if(strcmp(dst,"gpio1") == 0){
                _eq.push(new Event(EVT_GPIO1_TRIGERRED));
            }
        }
    }

    close (_sock1);
}
