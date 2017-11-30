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
#include <jmorecfg.h>
#include <sys/types.h>
#include <stdbool.h>

#define MAXLINE     1024
#define SA  struct sockaddr
#define LISTENQ     1024

#define CONNECT 1
#define CONNACK 2
#define PUBLISH 3
#define PUBACK 4
#define PUBREC 5
#define PUBREL 6
#define PUBCOMP 7
#define SUBSCRIBE 8
#define SUBACK 9
#define UNSUBSCRIBE 10
#define UNSUBACK 11
#define PINGREQ 12
#define PINGRESP 13
#define DISCONNECT 14

struct Accounts {
    char name[50];
    char pwd[50];
};

struct Clients {
    int socket;
    char username[50];
};

int get_account(struct Accounts []);

struct Accounts decode_packet_connect(char *buff);

bool authentication(struct Accounts, struct Accounts acc[], int n);

void create_account(struct Accounts a);

#endif //PROJECT_DEFINITION_H
