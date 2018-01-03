//
// Created by son on 29/11/2017.
//
#include "definition.h"

char *packet_connect(char *name, char *pwd) {
    size_t len;
    len = HEADER_LEN + strlen(name) + 1 + strlen(pwd);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = CONNECT << 4;
    strcat(pp, name);
    strcat(pp, ",");
    strcat(pp, pwd);
    return packet;
}

char *packet_subcribe(char *name, char *pwd) {
    size_t len;
    len = HEADER_LEN + strlen(name) + 1 + strlen(pwd);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = SUBSCRIBE << 4;
    strcat(pp, name);
    strcat(pp, ",");
    strcat(pp, pwd);
    return packet;
}

char *packet_publish(char *target, char *msg, int opt) {
    size_t len;
    len = HEADER_LEN + strlen(target) + 1 + strlen(msg);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = PUBLISH << 4;
    *pp |= opt;
    strcat(pp, target);
    strcat(pp, ".");
    strcat(pp, msg);
    return packet;
}

char *packet_connack(int y, int x) {
    fflush(stdout);
    char *packet = malloc(1);
    if (x == CONNACK) {
        *packet = CONNACK << 4;
    }
    if (x == SUBACK) {
        *packet = SUBACK << 4;
    }
    *packet |= y;
    return packet;
}

int decode_packet_connack(char *packet) {
    return *packet & 15;
}

void decode_packet_pubrec(char *msg) {
    msg = msg + 1;
    printf("\r%s\n", msg);
    printf(">>>");
    fflush(stdout);
}

char *packet_create(char *name) {
    size_t len;
    len = HEADER_LEN + strlen(name);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = CREATE << 4;
    strcat(pp, name);
    return packet;
}

char *packet_sendfile(char *target, char *file, int opt) {
    size_t len;
    len = HEADER_LEN + strlen(target) + 1 + strlen(file);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = SENDFILE << 4;
    *pp |= opt;
    strcat(pp, target);
    strcat(pp, ".");
    strcat(pp, file);
    return packet;
}

char *packet_pubrec(char *msg) {
    size_t len;
    len = HEADER_LEN + strlen(msg);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = PUBREC << 4;
    strcat(pp, msg);
    return packet;
}

char *packet_add(int sock, char *room, char *user) {
    size_t len;
    len = HEADER_LEN + strlen(room) + 1 + strlen(user);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = ADD << 4;
    strcat(pp, room);
    strcat(pp, ".");
    strcat(pp, user);
    return packet;
}

void printProgress(double process) {
    int val = (int) (process * 100);
    int lpad = (int) (process * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

void process_recv_file(int sockfd, char *filename) {
    ssize_t n;
    char recvbuff[MAXLINE];
    FILE *f;
    char buffer[MAXLINE];
    int file_size, remain = 0;
    memset(recvbuff, 0, MAXLINE);
    memset(buffer, 0, MAXLINE);
    read(sockfd, recvbuff, MAXLINE);
    file_size = atoi(recvbuff);
    if (file_size == 0) {
        printf("%s\n", recvbuff);
    } else {
        f = fopen(filename, "w+");
        if (f == NULL) {
            perror("Can't open file");
        }
        while (remain < file_size) {
            n = read(sockfd, buffer, MAXLINE);
            fwrite(buffer, sizeof(char), n, f);
            remain += n;
            printProgress((double) remain / file_size);
        }
        fclose(f);
        if (remain == file_size) {
            printf("\nComplete\n");
        }
    }
}

void process_send_file(int sockfd, char *filename) {
    FILE *fd;
    char buffer[MAXLINE];
    char sendbuff[MAXLINE];
    ssize_t n;
    int file_size, remain = 0;
    memset(sendbuff, 0, MAXLINE);
    fd = fopen(filename, "r+");
    if (fd == NULL) {
        printf("File does not exist\n");
        strcpy(sendbuff, "File does not exist");
        write(sockfd, sendbuff, sizeof(sendbuff));
    } else {
        memset(buffer, 0, MAXLINE);
        fseek(fd, 0, SEEK_END);
        file_size = ftell(fd);
        sprintf(sendbuff, "%d", file_size);
        write(sockfd, sendbuff, sizeof(sendbuff));
        rewind(fd);
        while (remain < file_size) {
            n = fread(buffer, sizeof(char), 1024, fd);
            remain += n;
            write(sockfd, buffer, n);
            printProgress((double) remain / file_size);
        }
        fclose(fd);
        if (remain == file_size) {
            printf("\nComplete\n");
        }
    }
}