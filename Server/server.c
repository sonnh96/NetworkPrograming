//
// Created by son on 29/11/2017.
//

#include "definition.h"

struct Accounts acc[100];
int noacc = 0;

struct Clients clients[100];
int nocl = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void set_connect(int sock, char *buff) {
    struct Accounts a = decode_packet_connect(buff);
    if (authentication(a, acc, noacc)) {
        printf("Connect success!");
        pthread_mutex_lock(&mutex);
        clients[nocl].socket = sock;
        strcpy(clients[nocl].username, a.name);
        nocl++;
        pthread_mutex_unlock(&mutex);
    } else {
        printf("Disconnect!");
        close(sock);
    }
}

void subscribe(int sockfd, char *buff) {
    printf("Subcribe");
    struct Accounts a = decode_packet_connect(buff);
    create_account(a);
//    pthread_mutex_lock(&mutex);
//    noacc = get_account(acc);
//    pthread_mutex_unlock(&mutex);
}

void *handle(void *iptr) {
    int sockfd = *(int *) iptr;
    char buff[MAXLINE];

    while (1) {
        memset(buff, 0, MAXLINE);
        if (read(sockfd, buff, 1024) < 0) {
            break;
        }
        if (buff[strlen(buff) - 1] == '\n')
            buff[strlen(buff) - 1] = '\0';
        if (buff[strlen(buff) - 1] == '\r')
            buff[strlen(buff) - 1] = '\0';

        int type = (*buff >> 4) & 15;
        switch (type) {
            case CONNECT:
                set_connect(sockfd, buff);
                break;
            case SUBSCRIBE:
                subscribe(sockfd, buff);
                break;
            default:
                break;
        }
    }

    return 0;
}

int main(int argc, char const *argv[]) {

    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    pthread_t tid;
    int *iptr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        perror("ERROR opening socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9966);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(listenfd, LISTENQ);

    printf("Server is listening at port %d\n", servaddr.sin_port);

    noacc = get_account(acc);

    while (1) {
        iptr = malloc(sizeof(int));
        clilen = sizeof(cliaddr);
        *iptr = accept(listenfd, (SA *) &cliaddr, &clilen);
        char s[255];
        inet_ntop(cliaddr.sin_family, (struct sockaddr *) &cliaddr, s, sizeof s);
        printf("Server get connection from %s\n", s);
        pthread_create(&tid, NULL, &handle, (void *) iptr);
    }

}