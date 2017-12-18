//
// Created by son on 29/11/2017.
//
#include "definition.h"

int sockfd;
char username[50], password[50];

void recv_file(char *buff) {
    buff = buff + 1;
    printf("Recv: %sq", buff);
    process_recv_file(sockfd, buff);
}

void download_file(int sock) {
    char file[50];
    size_t len;
    printf("Enter file name: ");
    fflush(stdin);
    fgets(file, 50, stdin);
    file[strlen(file) - 1] = '\0';
    len = HEADER_LEN + strlen(file);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = DOWNFILE << 4;
    strcat(pp, file);
    write(sock, packet, strlen(packet));
    free(packet);
}

void send_file(int sock) {
    int opt;
    char target[50], file[50], *buff;
    memset(target, 0, 50);
    memset(file, 0, 50);
    LOOP:
    printf("Type \"0\" to send to user\n");
    printf("Type \"1\" to send to room\n");
    printf(">>>");
    scanf("%d", &opt);
    getchar();
    if (opt == 0) {
        printf("Enter name of user: ");
        fflush(stdin);
        fgets(target, 50, stdin);
        target[strlen(target) - 1] = '\0';
    } else if (opt == 1) {
        printf("Enter name of room: ");
        fflush(stdin);
        fgets(target, 50, stdin);
        target[strlen(target) - 1] = '\0';
    } else {
        goto LOOP;
    }
    printf("Enter name of file: ");
    fflush(stdin);
    fgets(file, 50, stdin);
    file[strlen(file) - 1] = '\0';
    buff = packet_sendfile(target, file, opt);
    write(sock, buff, strlen(buff));
    usleep(200);
    process_send_file(sock, file);
}

void add_user(int sock) {
    char room[50], users[MAXLINE];
    char *sendbuff;
    printf("Enter room to add user: ");
    fflush(stdin);
    fgets(room, 50, stdin);
    room[strlen(room) - 1] = '\0';
    printf("Enter list user(user1,user2,...): ");
    fflush(stdin);
    fgets(users, MAXLINE, stdin);
    users[strlen(users) - 1] = '\0';
    sendbuff = packet_add(sock, room, users);
    write(sock, sendbuff, strlen(sendbuff));
}

void create_room(int sock) {
    char name[50];
    char *sendbuff;
    printf("Enter name of room: ");
    fflush(stdin);
    fgets(name, sizeof(name), stdin);
    name[strlen(name) - 1] = '\0';
    sendbuff = packet_create(name);
    write(sock, sendbuff, strlen(sendbuff));
}

void chat(int sock) {
    int opt;
    char target[50], msg[MAXLINE], *buff;
    LOOP:
    printf("Type \"0\" to send to user\n");
    printf("Type \"1\" to send to room\n");
    printf(">>>");
    scanf("%d", &opt);
    getchar();
    if (opt == 0) {
        printf("Enter name of user: ");
        fflush(stdin);
        fgets(target, 50, stdin);
        target[strlen(target) - 1] = '\0';
    } else if (opt == 1) {
        printf("Enter name of room: ");
        fflush(stdin);
        fgets(target, 50, stdin);
        target[strlen(target) - 1] = '\0';
    } else {
        goto LOOP;
    }
    printf("Enter messenger: ");
    fflush(stdin);
    fgets(msg, MAXLINE, stdin);
    msg[strlen(msg) - 1] = '\0';
    buff = packet_publish(target, msg, opt);
    printf("Chat: %s\n", buff);
    write(sock, buff, strlen(buff));
    fflush(stdout);
}

void get_list(int connfd) {
    char *packet = malloc(1);
    *packet = GET << 4;
    write(connfd, packet, strlen(packet));
}

void check_ack(char *buff) {
    int c;
    c = decode_packet_connack(buff);
    if (c == 0) {
        printf("Fail!\n");
        exit(1);
    } else {
        printf(".-------------------------------------------------.\n");
        printf("|1. Type: \"chat\" to chat                          |\n");
        printf("|2. Type: \"get\" to get list users and rooms       |\n");
        printf("|3. Type: \"create\" to create room                 |\n");
        printf("|4. Type: \"add\" to add user to the room           |\n");
        printf("|5. Type: \"sendfile\" to send file                 |\n");
        printf("|6. Type: \"downfile\" to download file             |\n");
        printf("|7. Type: \"exit\" to exit program                  |\n");
        printf("*-------------------------------------------------*\n");
        printf(">>>");
    }
    fflush(stdout);
}

void send_connect(int sock) {
    printf("Enter username and password to sign in!\n");
    printf("Username: ");
    fgets(username, 50, stdin);
    fflush(stdin);
    printf("Password: ");
    fgets(password, 50, stdin);
    fflush(stdin);
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';
    char *buff = packet_connect(username, password);
    write(sock, buff, strlen(buff));
}

void send_subscribe(int sock) {
    printf("Sign up an account!\n");
    printf("Username: ");
    fflush(stdin);
    fgets(username, 50, stdin);
    printf("Password: ");
    fflush(stdin);
    fgets(password, 50, stdin);
    username[strlen(username) - 1] = '\0';
    password[strlen(password) - 1] = '\0';
    char *buff = packet_subcribe(username, password);
    write(sock, buff, strlen(buff));
}

void *recv_msg(void *sockfd) {
    int connfd = *((int *) sockfd);
    char *buff;
    while (1) {
        buff = malloc(MAXLINE);
        if (read(connfd, buff, MAXLINE) <= 0) {
            close(connfd);
            break;
        } else {
            if (buff[strlen(buff) - 1] == '\n')
                buff[strlen(buff) - 1] = '\0';
            if (buff[strlen(buff) - 1] == '\r')
                buff[strlen(buff) - 1] = '\0';
            int type = (*buff >> 4) & 15;
            switch (type) {
                case CONNACK:
                    check_ack(buff);
                    continue;
                case SUBACK:
                    check_ack(buff);
                    continue;
                case PUBREC:
                    decode_packet_pubrec(buff);
                    continue;
                case SENDFILE:
                    recv_file(buff);
                    continue;
                default:
                    break;
            }
        }
        free(buff);
    }
    return 0;
}

void *send_msg(void *sockfd) {
    int connfd = *((int *) sockfd);
    char option[10];
    while (1) {
        fflush(stdin);
        fgets(option, sizeof(option), stdin);
        option[strlen(option) - 1] = '\0';
        if (strcmp(option, "chat") == 0) {
            chat(connfd);
        } else if (strcmp(option, "get") == 0) {
            get_list(connfd);
        } else if (strcmp(option, "create") == 0) {
            create_room(connfd);
        } else if (strcmp(option, "sendfile") == 0) {
            send_file(connfd);
        } else if (strcmp(option, "downfile") == 0) {
            download_file(connfd);
        } else if (strcmp(option, "add") == 0) {
            add_user(connfd);
        } else if (strcmp(option, "exit") == 0) {
            exit(1);
        }
        printf(">>>");
    }
    return 0;
}

int main(int argc, char const *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Enter hostname\n");
        exit(0);
    }
    struct sockaddr_in servaddr;
    pthread_t recvt, send;

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

    char check[5];
    while (1) {
        printf("Do you have account(yes/no)!");
        fgets(check, 10, stdin);
        fflush(stdin);
        check[strlen(check) - 1] = '\0';
        if (strcmp(check, "yes") == 0) {
            send_connect(sockfd);
            break;
        } else if (strcmp(check, "no") == 0) {
            send_subscribe(sockfd);
            break;
        }
    }
    pthread_create(&recvt, NULL, recv_msg, &sockfd);
    pthread_create(&send, NULL, send_msg, &sockfd);
    pthread_join(recvt, NULL);
    pthread_join(send, NULL);
    close(sockfd);

    return 0;
}