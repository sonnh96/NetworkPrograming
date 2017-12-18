//
// Created by son on 29/11/2017.
//
#include "definition.h"

int get_account(struct Accounts acc[]) {
    int n;
    FILE *fp = fopen("Users.json", "r");
    char line[100];
    char *token, *token1;
    while (fgets(line, sizeof line, fp) != NULL) {
        if (strstr(line, "\"username\"") != NULL) {
            token = strtok(line, ":");
            token = strtok(NULL, ":");
            token1 = strtok(token, "\"");
            token1 = strtok(NULL, "\"");
            strcpy(acc[n].name, token1);
            fgets(line, sizeof line, fp);
            token = strtok(line, ":");
            token = strtok(NULL, ":");
            token1 = strtok(token, "\"");
            token1 = strtok(NULL, "\"");
            strcpy(acc[n].pwd, token1);
            n++;
        }
    }
    fclose(fp);
    return n;
}

bool create_account(struct Accounts a, struct Accounts acc[], int n) {
    int i;
    for (i = 0; i < n; i++) {
        if ((strcmp(acc[i].name, a.name) == 0))
            return false;
    }
    FILE *f = fopen("Users.json", "a+");
    fseeko(f, -2, SEEK_END);
    int position = ftello(f);
    ftruncate(fileno(f), position);
    fprintf(f, ",\n");
    fprintf(f, "\t{\"username\" : \"");
    fprintf(f, "%s", a.name);
    fprintf(f, "\",\n");
    fprintf(f, "\t\"password\" : \"");
    fprintf(f, "%s", a.pwd);
    fprintf(f, "\"}\n]");
    fclose(f);
    return true;
}

struct Accounts decode_packet_connect(char *buff) {
    char *p = buff;
    p = p + 1;
    struct Accounts a;
    strcpy(a.name, strtok(p, ","));
    if (a.name[strlen(a.name) - 1] == '\n')
        a.name[strlen(a.name) - 1] = '\0';
    strcpy(a.pwd, strtok(NULL, ","));
    if (a.name == NULL || a.pwd == NULL) {
        strcpy(a.name, "1");
        strcpy(a.pwd, "1");
    }
    return a;
}

bool authentication(struct Accounts a, struct Accounts acc[], int n) {
    int count = 0;
    int i = 0;
    for (i = 0; i < n; i++) {
        if ((strcmp(acc[i].name, a.name) == 0) && (strcmp(acc[i].pwd, a.pwd) == 0)) {
            count++;
        }
    }
    if (count == 0)
        return false;
    else
        return true;
}

char *packet_connack(int y, int x) {
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

char *packet_fileack(int x) {
    char *packet = malloc(1);
    *packet |= x;
    return packet;
}

int decode_packet_connack(char *packet) {
    return *packet & 15;
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

bool chat_user(int sockfd, struct Clients clients[], int n, char *buff, bool x) {
    buff = buff + 1;
    char *msg, *target;
    char send[MAXLINE];
    memset(send, 0, MAXLINE);
    target = strtok(buff, ".");
    msg = strtok(NULL, "");
    printf("1.%s.%s\n", target, msg);
    int i, des = 0;
    for (i = 0; i < n; i++) {
        if (sockfd == clients[i].socket) {
            strcat(send, clients[i].username);
        }
        if (strcmp(target, clients[i].username) == 0) {
            des = clients[i].socket;
        }
    }
    if (x) {
        strcat(send, " send file:");
        process_recv_file(sockfd, msg);
    } else {
        strcat(send, ": ");
    }
    strcat(send, msg);
    strcpy(send, packet_pubrec(send));
    printf("Chat: %s\n", send);
    fflush(stdout);
    if (des != 0) {
        write(des, send, strlen(send) + 1);
        free(packet_pubrec(send));
        memset(send, 0, MAXLINE);
        return true;
    } else {
        return false;
    }
}

bool chat_room(int sockfd, struct Rooms rooms[], int n, struct Clients clients[], int n1, char *buff, bool x) {
    buff = buff + 1;
    char *msg, *target;
    char send[MAXLINE];
    memset(send, 0, MAXLINE);
    target = strtok(buff, ".");
    msg = strtok(NULL, "");
    printf("1.%s.%s\n", target, msg);
    int i, j, k, des = 0;
    for (i = 0; i < n1; i++) {
        if (sockfd == clients[i].socket) {
            strcat(send, clients[i].username);
            break;
        }
    }
    if (x) {
        strcat(send, " send file to ");
        process_recv_file(sockfd, msg);
    } else {
        strcat(send, " send to ");
    }
    strcat(send, target);
    strcat(send, ": ");
    strcat(send, msg);
    strcpy(send, packet_pubrec(send));
    printf("Chat: %s\n", send);
    fflush(stdout);
    for (i = 0; i < n; i++) {
        if (strcmp(target, rooms[i].name) == 0) {
            for (j = 0; j < n1; j++) {
                for (k = 0; k < rooms[i].count; k++) {
                    if (strcmp(rooms[i].list[k], clients[j].username) == 0 && clients[j].socket != sockfd) {
                        des = clients[j].socket;
                        write(des, send, strlen(send) + 1);
                    }
                }
            }
            break;
        }
    }
    free(packet_pubrec(send));
    memset(send, 0, MAXLINE);
    if (des != 0) {
        return true;
    } else
        return false;
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
        printf("File size: %d\n", file_size);
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