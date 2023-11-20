#include "csapp.h"
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>


/*******************************
 * Protocol-independent wrappers
 *******************************/
/* $begin getaddrinfo */
void Getaddrinfo(const char *node, const char *service, 
                 const struct addrinfo *hints, struct addrinfo **res)
{
    int rc;

    if ((rc = getaddrinfo(node, service, hints, res)) != 0) 
        gai_error(rc, "Getaddrinfo error");
}
/* $end getaddrinfo */

void Getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, 
                 size_t hostlen, char *serv, size_t servlen, int flags)
{
    int rc;

    if ((rc = getnameinfo(sa, salen, host, hostlen, serv, 
                          servlen, flags)) != 0) 
        gai_error(rc, "Getnameinfo error");
}

void Freeaddrinfo(struct addrinfo *res)
{
    freeaddrinfo(res);
}


/* $begin open_clientfd */
/* custmized function, implementing client-side socket using getaddrinfo(in <netdb.h>)*/
/* open_clientfd establish a connection with a server 
    (1)running oh host(hostname) and 
    (2)listening for connection req on port number port*/

/* will return an open socket descriptor(=int number) which is ready for I/O using the Unix I/O funcs.*/
int open_clientfd(char *hostname, char *port) {
    
    int clientfd, rc; //rc means "return code"
    struct addrinfo hints, *listp, *p; 
    //addrinfo : structure to contain info about addr of a service provider.
    // *listp, *p 는 addrinfo 구조체로 쓸 것들

    /* Get a list of potential server addresses */

    memset(&hints, 0, sizeof(struct addrinfo));
    /* struct addrinfo hint의 주소에 
        struct addrinfo's size bytes 의 메모리 공간을 
        constant byte 0 만큼으로 채운다.*/

    /* Open a connection */
    /* hints: user가 이미 만들어진 addrinfo 구조체 안의 
    속성들에 맞게 필요한 검색어(?)를 넣어 커스텀한다. */
    hints.ai_socktype = SOCK_STREAM;  // steam : TCP로 찾겠다.
    hints.ai_flags = AI_NUMERICSERV;  /* ... using a numeric port arg. */
    hints.ai_flags |= AI_ADDRCONFIG;  /* Recommended for connections */

    /*
    getaddrinfo returns a list of addrinfo structures, 
    each of which points to a socket address structure is suitable for 
    establishing a connection with a server running on 
    (the parameter)hostname and listening on (the parameter)port.
    */ 
    if ((rc = getaddrinfo(hostname, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", hostname, port, gai_strerror(rc));
        return -2;
    }
  
    /* Walk the list for one that we can successfully connect to */
    /*rc는 getaddrinfo의 fd(즉, 정수)가 담겨있고 
    hint에 있는 addrinfo 구조체 형태의 커스텀 검색어로 찾아 받아온 ip 정보는
    listp에 있는 addrinfo 구조체 형태에 정보가 저장되어 있다. 
   */
    
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        /*   
        p는 그 받아와 저장된 정보를 'walk' 하며 넣은 것이며 
        그 정보는 다발(list)형태라 loop로 순회하며 socket 함수의 파라미터로서 사용해 
        소켓을 만들어 clientfd에 넣는다. 
        */
        
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue; /* Socket failed, try the next */

        /* Connect to the server */
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1) 
            break; /* Success */
        if (close(clientfd) < 0) { /* Connect failed, try another */  
        //line:netp:openclientfd:closefd
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        } 
    } 

    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* All connects failed */
        return -1;
    else    /* The last connect succeeded */
        return clientfd;
}
/* $end open_clientfd */

/*  
 * open_listenfd - Open and return a listening socket on port. This
 *     function is reentrant and protocol-independent.
 *
 *     On error, returns: 
 *       -2 for getaddrinfo error
 *       -1 with errno set for other errors.
 */
/* $begin open_listenfd */
int open_listenfd(char *port) 
{
    struct addrinfo hints, *listp, *p;
    int listenfd, rc, optval=1;

    /* Get a list of potential server addresses */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;             /* Accept connections */
    hints.ai_flags |= AI_NUMERICSERV;            /* ... using port number */

    /* 
     [ Listen ] 

    Since we have called getaddrinfo with the AI_PASSIVE ﬂag and a NULL host argument, 
    the address ﬁeld in each socket address structure is set to the wildcard address, 
    which tells the kernel that this server will accept requests to 
    any of the IP addresses for this host.
    */
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; /* ... on any IP address */
  

    if ((rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo failed (port %s): %s\n", port, gai_strerror(rc));
        return -2;
    }

    /* Walk the list for one that we can bind to */
    for (p = listp; p; p = p->ai_next) {
        /* Create a socket descriptor */
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) 
            continue;  /* Socket failed, try the next */

        /* 
        We call getaddrinfo and then walk the resulting list 
        until the calls to socket and bind succeed. 
        
        we use the setsockopt function (not described here) to conﬁgure the server so that it can be terminated, be restarted, and begin accepting connection requests immediately.
       
        By default, a restarted server will deny connection requests from 
        clients for approximately 30 seconds, which seriously hinders debugging.
       */


        /* Eliminates "Address already in use" error from bind */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,    //line:netp:csapp:setsockopt
                   (const void *)&optval , sizeof(int));

        /* Bind the descriptor to the address */
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; /* Success */
        if (close(listenfd) < 0) { /* Bind failed, try the next */
            fprintf(stderr, "open_listenfd close failed: %s\n", strerror(errno));
            return -1;
        }
    }


    /* Clean up */
    freeaddrinfo(listp);
    if (!p) /* No address worked */
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    /* to convert listenfd to a listening descriptor and return it to the caller.*/

    if (listen(listenfd, LISTENQ) < 0) {
        close(listenfd);

        /* to avoid a memory leak 
        by closing the descriptor before returning */
	return -1;
    }
    return listenfd;
}
/* $end open_listenfd */


/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
/* $begin rio_readinitb */
void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}
/* $end rio_readinitb */


//그..입력받는거 어떻게하더라 

// char input_txt;
// scanf("%c", &input_txt);

// char* input_txt_2;
// scanf("%s", input_txt_2);

// char* input_txt_2;
// printf("문자를 입력하세요: ");
// scanf("%s", input_txt_2);
// printf("%s\n", input_txt_2);

// return 0;

// //일단 입력 받고 
// char* standard_inputs;
// scanf("%s", standard_inputs);
// //입력된 text line을 읽어야 함
// // 이 text라인을 서버에 보내야 함; 
// // 서버에서 보내준 에코 라인을 읽어야 함 
// // 이 결과를 아웃풋으로 프린트 해야 함
void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
} 

