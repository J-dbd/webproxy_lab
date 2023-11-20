#include "csapp.h"

int main(int argc, char **argv){

    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc !=2){
        fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
        exit(0);
    }
    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    /* Walk the list and display each IP address */
    flags = NI_NUMERICHOST; /* Display address string instead of domain name */

    for (p = listp; p; p = p->ai_next){
        getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);

        
    }

    /* Clean up */
    freeaddrinfo(listp);

    exit(0);
}

/* on terminal */

// ubuntu@ip-172-31-8-63:~/week6$ ./hostinfo twitter.com
// 104.244.42.1
// 104.244.42.65
// 104.244.42.129
// 104.244.42.193