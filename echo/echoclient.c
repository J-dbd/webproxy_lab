/* Echo - client  */
#include "csapp.h"
// #include "csapp.c"
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>

/* 

[1] Establishing a connection 
[2] After establishing a connection with the server, the client ....

    1) enters a loop that repeatedly reads a text line from standard input, 
    2) sends the text line to the server, 
    3) reads the echo line from the server, and 
    4) prints the result to standard output. 

[3] The loop terminates when fgets encounters EOF on standard input, either 
    (1) because the user typed Ctrl+D at the keyboard or 
    (2) because it has exhausted the text lines in a redirected input ﬁle.

[4] After the loop terminates, the client closes the descriptor.

[5] This results in an EOF notiﬁcation being sent to the server

[6] The server detects when it receives a return code of zero 
    from its rio_readlineb function.

[7] After closing its descriptor, the client terminates. 

    Since the client’s kernel automatically closes all open descriptors 
    when a process terminates, the close in line 24 is not necessary. 
    However, it is good programming practice to explicitly close any descriptors 
    that you have opened.

*/



int main(int argc, char** argv){

    
    int clientfd; //file descriptors 
    char *host, *port, buf[MAXLINE];
    rio_t rio; 
    
    //argv[0], argv[1], argv[2] 가 아니라면 오류 메시지 출력
    if(argc !=3){
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    /*
    ./echoclient localhost 8000
                 argv[1]  argv[2]
    */
    host = argv[1]; //char* host_터미널에서 입력받은 첫번째 파라미터
    port = argv[2]; //char* port_터미널에서 입력받은 두번째 파라미터

    
    //clientfd = Open_clientfd(host, port);

    //////////////////////////////////////////////////
    //// [1] Establishing a connection           /////
    //// instead using Open_clientfd(host, port) /////
    //////////////////////////////////////////////////

    /* open the clientfd(socket)  */

    // vals for GetAddrInfo //
    int rc; 
    struct addrinfo hints, *listp, *p; //검색어, 되돌려받을 정보들, 순회하며 찾을 것.
    memset(&hints, 0, sizeof(struct addrinfo));
    //hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP stream
    hints.ai_flags = AI_NUMERICSERV; //server의 port number를 numeric하게 쓰겠다. 
    hints.ai_flags |= AI_ADDRCONFIG; /* Recommended for connections */

    //addrinfo 채우기
    if((rc = getaddrinfo(host, port, &hints, &listp) != 0)){
        fprintf(stderr, "getaddrinfo failed (%s:%s): %s\n", host, port, gai_strerror(rc));
        return -2;
    }

    /* connect the socket with server*/

    for( p = listp; p; p = p->ai_next){

        if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <0){
            continue; // Socket failed, try the next
        }

        //여기서부터는 연결이 성공, connect한다. 
        if((connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)){
            break; // 성공하면 break!
        }
        
        if((close(clientfd))<0){
            //실패했다면 어디서 실패했는지 적어주고 try anohter한다.
            fprintf(stderr, "open_clientfd: close failed: %s\n", strerror(errno));
            return -1;
        }
    }
    /* Clean up */
    freeaddrinfo(listp);
    if(!p) return -1; /* All connects failed */
    

///////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////
    //// [2] enter loop until fgets encounters EOF /////
    ////////////////////////////////////////////////////

    /* 
    initialize the buffer that will be used for communication back and forth for this network communication
    */ 

    /* 
    [rio_readinitb]
    Associate a descriptor with a read buffer and reset buffer
    */
    Rio_readinitb(&rio, clientfd);

    /* 
    buf에 MAXLINE만큼 Standard input stream stdin을 통해 
    입력 받은 값을 넣는다. NULL이 될 때까지. 
    */
    while(Fgets(buf, MAXLINE, stdin) != NULL){
        Rio_writen(clientfd, buf, strlen(buf)); //write to network
        Rio_readlineb(&rio, buf, MAXLINE); //read from network
        Fputs(buf, stdout);
    }

    Close(clientfd);
    exit(0);

}


#ifdef USEINSIDECODE
//////////////////////////////////////////////////////////////////////////////

////////////////////////
//// fgets & fputs /////
////////////////////////

//// Fgets = fgets ////

char *Fgets(char *ptr, int n, FILE *stream) 
{
    char *rptr;
    if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
	app_error("Fgets error");

    return rptr;
}

/*

#include <stdio.h>
char *fgets (char *string, int n, FILE *stream);

Get a newline-terminated string of finite length from STREAM.

(ref: IBM)
fgets() 함수는 현재 stream 위치에서 어느 것이 먼저 오건 첫 번째 줄 바꾸기 문자(\n)까지, 스트림의 끝까지 또는 읽은 문자 수가 n-1과 같을 때까지 문자를 읽습니다. fgets() 함수는 결과를 string에 저장하고 스트링 끝에 널(null) 문자(\0)를 추가합니다. string은 줄 바꾸기 문자를 포함합니다(읽은 경우). n이 1이면 string이 비어 있습니다.

*/

// Fgets = fputs ////
void Fputs(const char *ptr, FILE *stream) 
{
    if (fputs(ptr, stream) == EOF)
	unix_error("Fputs error");
}

/*

#include <stdio.h>
int fputs(const char *string, FILE *stream);

Write a string to STREAM.

(ref:IBM)
fputs() 함수는 string을 현재 위치의 출력 stream으로 복사합니다. 스트링 끝의 null 문자 (\0)를 복사하지 않습니다.

*/



////////////////////////
//// stdin, stdout /////
////////////////////////

//// stdin ////

/* in <stdio.h> ...

// Standard streams.
// extern FILE *stdin;		// Standard input stream.
// extern FILE *stdout;		// Standard output stream. 
// extern FILE *stderr;		// Standard error output stream. 

// C89/C99 say they're macros.  Make them happy.
// #define stdin stdin    [ USED ]
// #define stdout stdout  [ USED ]
// #define stderr stderr

*/

////////////////////////
////  Rio Packages  ////
////////////////////////

//// rio_writen ////
// rio_writen - Robustly write n bytes (unbuffered)


void Rio_writen(int fd, void *usrbuf, size_t n) 
{
    if (rio_writen(fd, usrbuf, n) != n)
	unix_error("Rio_writen error");
}

/* $begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}
/* $end rio_writen */


//// rio_readlineb ////
//rio_readlineb - Robustly read a text line (buffered)

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
	unix_error("Rio_readlineb error");
    return rc;
} 


/* $begin rio_readlineb */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}
/* $end rio_readlineb */

#endif