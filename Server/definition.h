//
// Created by son on 29/11/2017.
//

#ifndef PROJECT_DEFINITION_H
#define PROJECT_DEFINITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAXLINE     1024
#define SA  struct sockaddr
#define LISTENQ     1024

#define MAXLINE     1024
#define SA  struct sockaddr
#define LISTENQ     1024
#define HEADER_LEN 1
#define CONNECT 1
#define CONNACK 2
#define PUBLISH 3
#define GET 4
#define PUBREC 5
#define CREATE 6
#define ADD 7
#define SUBSCRIBE 8
#define SUBACK 9
#define SENDFILE 10
#define DOWNFILE 11

struct Accounts {
    char name[50];
    char pwd[50];
};

struct Clients {
    int socket;
    char username[50];
};

struct Rooms {
    char name[50];
    int count;
    char *list[50];
};

int get_account(struct Accounts []);

struct Accounts decode_packet_connect(char *buff);

bool authentication(struct Accounts, struct Accounts acc[], int n);

bool create_account(struct Accounts a, struct Accounts acc[], int n);

char *packet_connack(int retCode, int x);

char *packet_add(int, char, char);

char *packet_pubrec(char *msg);

bool chat_user(int sockfd, struct Clients [], int, char *, bool);

bool chat_room(int sockfd, struct Rooms [], int, struct Clients [], int, char *, bool);

int decode_packet_connack(char *packet);

void decode_packet_pubrec(char *msg);

bool process_recv_file(int, char *);

bool process_send_file(int, char *);

#endif //PROJECT_DEFINITION_H
