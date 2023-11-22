
#include "csapp.h"
//#include "csapp.c"

#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>
#include <stdio.h>


/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

// int main() {
//   printf("%s", user_agent_hdr);
//   return 0;
// }

/*
11.23 : 순차적 프록시 구현 중, user client가 보내는 정보 저장 및 순회 조회까지 확인됨. 
next-> proxy 내부의 client socket을 열고 다른 web server에 요청을 보내고 조회하는 것. 
*/

////////////////////////
// 구조체 설정 및 선언 //
////////////////////////

//구조체와 리스트 선언


//정보를 담는 구조체
typedef struct user_req {
  char method;
  char uri;
  char version;
} user_req ;

// 구조체 node
typedef struct user_req_node {
  user_req data;
  struct user_req_node* next;
} user_req_node;

//proxy server가 keep해야 하는 유저 정보를 담는 linked list
user_req_node *user_req_list = NULL;

/* 사용 함수 선언
모든 함수들은 선언 순서대로 하단부에 적혀 있다. */

//main functions
void echo(int fd); //임시 이름
void proxy_server(char* port);

// sub functions 
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);
void read_requesthdrs(rio_t *rp);
void addUserRequest(char method, char uri, char version);
//void printHttpRes(user_req_node *current, int fd);


///////////////////
// main function //
///////////////////

int main(int argc, char** argv){
  //사용 변수
  char* server_port = argv[1];
  
  printf("%s\n", user_agent_hdr);
  printf("port: %s\n", server_port);
  
  /* 1. 순차적 프록시 */
  proxy_server(server_port);

  return 0;

}

///////////////////
// sub functions //
///////////////////

void echo(int connfd){
    size_t n;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;
    user_req req;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

        printf("Request headers:\n"); 
        printf("%s", buf); //buf에 출력

        sscanf(buf, "%s %s %s", method, uri, version);

        if(strcasecmp(method, "GET")){ //get 외 out
          printf("cp! %s\n", method);
          clienterror(connfd, method, "501", "Not Implemented",
                "Tiny doesn't implement this method");
          return;
        }

        //if method is "GET", Tiny simply reads and ignores any of info in req headers.
        read_requesthdrs(&rio);
        addUserRequest(method, uri, version);
    }

    user_req_node *current = user_req_list;

    while(current != NULL){
      //확인을 위한 터미널 출력
      printf("Method: %c, URI: %c, Version: %c\n", current->data.method, current->data.uri, current->data.version);

      //실제 request header로 조립해봄
      //printHttpRes(current, fd);
      
      current = current->next;
    }

}


void proxy_server(char* port){
  printf("proxy_server is on...\n");
  int listenfd, connfd;
  socklen_t clientlen; 
  char client_hostname[MAXLINE], client_port[MAXLINE];
  struct sockaddr_storage clientaddr;

  listenfd = Open_listenfd(port);

  while(1){
    clientlen = sizeof(struct sockaddr_storage);
    connfd = accept(listenfd, (SA*)&clientaddr, &clientlen);

    int rc;
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


/* --------------------------------------------------

Tiny clienterror sends an error message to the client 

----------------------------------------------------- */

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
    char buf[MAXLINE], body[MAXBUF];

 /* Error msg를 화면에 출력하기 위해 
  응답에 쓰이는 the HTTP resonse body를 build함 */


    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));
    Rio_writen(fd, body, strlen(body));
}

/* -------------------------------------------------------

Tiny read_requesthdrs reads and ignores request headers. 

---------------------------------------------------------*/
void read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) { //다르면 while 안으로 들어옴
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}


// 새로운 유저 정보를 해당 구조체에 담고 그 구조체를 리스트에 담기 위한 함수
// rbtree때 생각남
void addUserRequest(char method, char uri, char version) {
    user_req_node *newNode = (user_req_node *)malloc(sizeof(user_req_node));
    if (!newNode) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    newNode->data.method = method;
    newNode->data.uri = uri;
    newNode->data.version = version;
    newNode->next = user_req_list;

    user_req_list= newNode;
}

// void printHttpRes(user_req_node *current, int fd){
//   char buf[MAXLINE];
//   /* print the HTTP request header*/
//   sprintf(buf, "%s/%s/%s)", current->data.method, current->data.uri, current->data.version);
//   Rio_writen(fd, buf, strlen(buf));
// }