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
    fflush(stdout);
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
    char *send = malloc(MAXLINE);
    target = strtok(buff, ".");
    printf("target: %s\n", target);
    msg = strtok(NULL, "");
    printf("msg: %s\n", msg);
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
    } else {
        strcat(send, ": ");
    }
    strcat(send, msg);
    printf("1: %s\n", send);
    send = packet_pubrec(send);
    printf("2: %s\n", send);
    if (x) {
        process_recv_file(sockfd, msg);
    }
    if (des != 0) {
        write(des, send, strlen(send) + 1);
        free(send);
        return true;
    } else{
        free(send);
        return false;
    }

}

bool chat_room(int sockfd, struct Rooms rooms[], int n, struct Clients clients[], int n1, char *buff, bool x) {
    buff = buff + 1;
    char *msg, *target;
    char *send = malloc(MAXLINE);
    target = strtok(buff, ".");
    printf("target: %s\n", target);
    msg = strtok(NULL, "");
    printf("msg: %s\n", msg);
    int i, j, k, des = 0;
    for (i = 0; i < n1; i++) {
        if (sockfd == clients[i].socket) {
            strcat(send, clients[i].username);
            break;
        }
    }
    if (x) {
        strcat(send, " send file to ");
    } else {
        strcat(send, " send to ");
    }
    strcat(send, target);
    strcat(send, ": ");
    strcat(send, msg);
    printf("1: %s\n", send);
    send = packet_pubrec(send);
    printf("2: %s\n", send);
    if (x) {
        process_recv_file(sockfd, msg);
    }
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
    free(send);
    if (des != 0) {
        return true;
    } else
        return false;
}

bool process_recv_file(int sockfd, char *filename) {
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
        }
        fclose(f);
    }
    if (remain == file_size) {
        printf("Recv complete\n");
        return true;
    } else {
        return false;
    }
}

bool process_send_file(int sockfd, char *filename) {
    FILE *fd;
    char buffer[MAXLINE];
    char sendbuff[MAXLINE];
    int file_size, remain = 0;
    ssize_t n;
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
        }
        fclose(fd);
    }
    if (remain == file_size) {
        printf("Send complete\n");
        return true;
    } else {
        return false;
    }
}