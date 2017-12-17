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
    char pswd[50];
    char username[50];
};

void get_account();

char *packet_connect(char *name, char *pwd);

char *packet_subcribe(char *name, char *pwd);

char *packet_publish(char *target, char *msg, int opt);

int decode_packet_connack(char *packet);

void decode_packet_pubrec(char *msg);

char *packet_create(char *name);

char *packet_sendfile(char *target, char *msg, int opt);

char *packet_pubrec(char *msg);

char *packet_add(int, char *, char *);

bool process_send_file(int, char *);

bool process_recv_file(int sockfd, char *filename);

#endif //PROJECT_DEFINITION_H
