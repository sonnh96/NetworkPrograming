//
// Created by son on 29/11/2017.
//
#include "definition.h"

void send_connect(int sock, char *name, char *pwd) {
    printf("Enter username and password to sign in!\n");
    printf("Username: ");
    fflush(stdin);
    fgets(name, 50, stdin);
    printf("Password: ");
    fflush(stdin);
    fgets(pwd, 50, stdin);
    name[strlen(name) - 1] = '\0';
    pwd[strlen(pwd) - 1] = '\0';
    char *buff = packet_connect(name, pwd);
    write(sock, buff, strlen(buff));
}

void send_subscribe(int sock, char *name, char *pwd) {
    printf("Sign up an account!\n");
    printf("Username: ");
    fflush(stdin);
    fgets(name, 50, stdin);
    printf("Password: ");
    fflush(stdin);
    fgets(pwd, 50, stdin);
    char *buff = packet_subcribe(name, pwd);
    write(sock, buff, strlen(buff));
}

//void *recv_msg(void *sockfd) {
//
//}
//
//void *send_msg(void *sockfd) {
//
//}

int main(int argc, char const *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Enter hostname\n");
        exit(0);
    }

    int sockfd;
    struct sockaddr_in servaddr;
    pthread_t recv, send;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9966);

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "Error: IP failed!\n");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        fprintf(stdout, "Error: Connection failed!\n");
        exit(1);
    }

    char username[50], password[50];
    char check[5];
    while (1) {
        printf("Do you have account(yes/no)!");
        fflush(stdin);
        fgets(check, 5, stdin);
        check[strlen(check) - 1] = '\0';
        if (strcmp(check, "yes") == 0) {
            send_connect(sockfd, username, password);
            break;
        } else if (strcmp(check, "no") == 0) {
            send_subscribe(sockfd, username, password);
            break;
        }
    }

    
    pthread_create(&recv, NULL, recv_msg, &sockfd);
    pthread_create(&send, NULL, send_msg, &sockfd);

    return 0;
}