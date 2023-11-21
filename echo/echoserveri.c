/* Echo - client  */
#include "csapp.h"
// #include "csapp.c"
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>

/*

[1] open the listening descriptor
[2] After opening the listening descriptor, it enters an inﬁnite loop.
    Each iteration...
    (1) waits for a connection request from a client, 
    (2) prints the domain name and port of the connected client, 
    (3) and then calls the echo function that services the client.
[3] the echo routine returns
[4] the main routine closes the connected descriptor. 
    Once the client and server have closed their respective descriptors, 
    the connection is terminated.

*/

void echo(int connfd);

int main(int argc, char** argv){

    

    int listenfd, connfd;

    // <-> echoclient
    // setting client's side element

    socklen_t clientlen; 
    char client_hostname[MAXLINE], client_port[MAXLINE], *port;
    struct sockaddr_storage clientaddr; /* Enough space for any address */

    if (argc !=2){
        fprintf(stderr,"usage: %s <port>\n", argv[0]);
        exit(0);
    }

    port = argv[1];
    //port = argv[2]; : 왜 client의 port는 저장하지 않을까?

    //listenfd = Open_listenfd(port);
    /////////////////[ setting listenfd ]//////////////////////////
    /* 1. setting vals */
    int rc, optval=1;
    struct addrinfo hints, *listp, *p;
    /* 2. setting hints */
    //Get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo)); //init
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; /* ... using port number */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */

    /* 3. fill struct addrinfo using getaddrinfo() */
    if ((rc = getaddrinfo(NULL, port, &hints, &listp) !=0)){
        fprintf(stderr, "getaddrinfo failed (port %s):%s\n", port, gai_strerror(rc));
        return -2;
    }
    /* 4. create a socket descriptor  */
    for(p = listp; p; p=p->ai_next){
        if((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <0){
            continue;
        }
        /*5. binding에서 '이미 사용중인 주소' 에러를 제거한다. */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
        //line:netp:csapp:setsockopt

        /* 6. binding */

        if(bind(listenfd, p->ai_addr, p->ai_addrlen) ==0){
            printf("success binding....\n");
            printf("waiting for connection....\n");
            break; // select and bind success!
        }
        // in client side .... listen
        // if(listen(listenfd, p->ai_addrlen)!=-1){
        //     break;
        // }
        if(close(listenfd)<0){
            /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }
    /* Clean up */
    freeaddrinfo(listp);
    if(!p) return -1; /* No address worked */

    /* 7. make listening socket ready to accept connection reqs. */ 
    if(listen(listenfd, LISTENQ)<0){
        close(listenfd);
        return -1;
    }

    /////////////////////////////////////

    

    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (SA*)&clientaddr, &clientlen);
        
        /* 
        1) 첫번째 인자(fd)에서 connection 이 발생하기 전까지 wait함
        2) connection이 발생하면 새로운 소켓을 연다.
        3) 두번째 인자(struct sockaddr*)를 이 연결 peer의 주소로 설정한다.(소통하는 상대방을 두번째 인자로 받은 주소라고 인지한다.)
        4) clientlen을 주소의 실제 길이로 설정한다.
        5) 새 소켓의 descriptor를 return (or return -1 on error)  */

        int rc;

        // SA = { tyepdef struct sockaddr SA in <csapp.h>}
        // sockaddr is in socket.h
        // strucutre describing a generic socket address.
        // clientaddr는 위에서 sockaddr_storage 타입으로 선언되었다. 
        // clientaddr의 주소(&clientaddr)를 SA*(= sockaddr*)로 캐스팅
        // getnameinfo함수는 socket address의 network address information을 
        // 사람이 읽을 수 있는 정보로 변환하는 것.
        // 따라서 맨 처음 파라미터에 정보를 얻고 싶은 sockaddr의 주소를 넣어 주어야 함.
        // Translate a socket address to a location and service name.
        // 그리고 미리 정의해둔 client_hostname/client_port에 읽어온 겂을 populate한다.

        if((rc = getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0)) != 0){
            fprintf(stderr, "getnameinfo error!: %s\n", gai_strerror(rc));
        }
        printf("Connected to (host: %s & port: %s)\n", client_hostname, client_port);
        echo(connfd);
        if ((rc = close(connfd))<0){
            unix_error("Close_error");
        }
        printf("connection lost.\n");
        exit(0);
    }


}

void echo(int connfd){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("client: %s", buf);
        printf("....server received %d bytes\n", (int)n);
        Rio_writen(connfd, buf, n);
    }

}

#ifdef USEINSIDECODE
///////////////////////////////////////


struct sockaddr_storage;
/* <socket.h>
Structure large enough to hold any socket address (with the historical exception of AF_UNIX).  */


void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags)
{
    int rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, 
                          servlen, flags)) != 0) 
        gai_error(rc, "Getnameinfo error");
}

void gai_error(int code, char *msg) /* Getaddrinfo-style error */
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(code));
    exit(0);
}

gai_strerror : <netdb.h>
#endif