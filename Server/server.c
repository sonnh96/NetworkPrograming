//
// Created by son on 29/11/2017.
//

#include "definition.h"

struct Accounts acc[100];
int noacc = 0;

struct Clients clients[100];
int nocl = 0;

struct Rooms rooms[100];
int nor = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void send_file(int sockfd, char *buff) {
    buff = buff + 1;
    printf("File: %s\n", buff);
    size_t len;
    len = HEADER_LEN + strlen(buff);
    char *packet = malloc(len);
    char *pp = packet;
    *pp = SENDFILE << 4;
    strcat(pp, buff);
    write(sockfd, packet, strlen(packet));
    usleep(200);
    process_send_file(sockfd, buff);
}

void recv_file(int sockfd, char *buff) {
    if ((*buff & 15) == 0) {
        chat_user(sockfd, clients, nocl, buff, true);
    } else if ((*buff & 15) == 1) {
        chat_room(sockfd, rooms, nor, clients, nocl, buff, true);
    }
}

void add_user(int sockfd, char *buff) {
    buff = buff + 1;
    int i;
    char *room, *token;
    room = strtok(buff,".");
    for (i = 0; i < nor; i++) {
        if(strcmp(room, rooms[i].name) == 0){
            token = strtok(NULL, ",");
            while( token != NULL )
            {
                rooms[i].list[rooms[i].count] = token;
                printf("Add: %s: %d", rooms[i].list[rooms[i].count], rooms[i].count);
                rooms[i].count++;
                fflush(stdout);
                token = strtok(NULL, ",");
            }
        }
    }
}

void create_room(int sockfd, char *buff) {
    buff = buff + 1;
    strcpy(rooms[nor].name, buff);
    printf("Hello: %s: %d", rooms[nor].name, nor);
    nor++;
    fflush(stdout);
}

void check_publish(int sockfd, char *buff) {
    if ((*buff & 15) == 0) {
        chat_user(sockfd, clients, nocl, buff, false);
    } else if ((*buff & 15) == 1) {
        chat_room(sockfd, rooms, nor, clients, nocl, buff, false);
    }
}

void get_list(int sock){
    char sendbuff[MAXLINE];
    char *msg;
    int i;
    memset(sendbuff, 0, MAXLINE);
    strcpy(sendbuff, "List users online: ");
    for(i = 0; i < nocl; i++) {
        if (clients[i].username != NULL)
        {
            strcat(sendbuff,clients[i].username);
            strcat(sendbuff,", ");
        }
    }
    strcat(sendbuff, "\nList rooms: ");
    for(i = 0; i < nor; i++) {
        strcat(sendbuff,rooms[i].name);
        strcat(sendbuff,", ");
    }
    msg = packet_pubrec(sendbuff);
    write(sock, msg, strlen(msg));
}
void set_connect(int sockfd, char *buff) {
    char *sb;
    struct Accounts a = decode_packet_connect(buff);
    if (authentication(a, acc, noacc)) {
        pthread_mutex_lock(&mutex);
        clients[nocl].socket = sockfd;
        strcpy(clients[nocl].username, a.name);
        nocl++;
        pthread_mutex_unlock(&mutex);
        sb = packet_connack(1, CONNACK);
        write(sockfd, sb, strlen(sb));
        fflush(stdout);
    } else {
        sb = packet_connack(0, CONNACK);
        write(sockfd, sb, strlen(sb));
        close(sockfd);
    }
    fflush(stdout);
}

void create_defaul_room(struct Rooms rooms[]) {
    strcpy(rooms[0].name, "room1");
    rooms[0].list[rooms[0].count] = "user1";
    rooms[0].count++;
    rooms[0].list[rooms[0].count] = "user2";
    rooms[0].count++;
    rooms[0].list[rooms[0].count] = "user3";
    rooms[0].count++;
}

void subscribe(int sockfd, char *buff) {
    struct Accounts a = decode_packet_connect(buff);
    pthread_mutex_lock(&mutex1);
    if (create_account(a, acc, noacc)) {
        acc[noacc] = a;
        noacc++;
        pthread_mutex_lock(&mutex);
        clients[nocl].socket = sockfd;
        strcpy(clients[nocl].username, a.name);
        nocl++;
        pthread_mutex_unlock(&mutex);
        write(sockfd, packet_connack(1, SUBACK), 1);
    } else {
        printf("Disconnect!");
        write(sockfd, packet_connack(0, SUBACK), 1);
        close(sockfd);
    }
    pthread_mutex_unlock(&mutex1);
    fflush(stdout);
}

void *handle(void *iptr) {
    int sockfd = *(int *) iptr;
    char *buff;
    while (1) {
        buff = malloc(MAXLINE);
        if (read(sockfd, buff, 1024) <= 0) {
            close(sockfd);
            int i,j;
            pthread_mutex_lock(&mutex);
            for(i = 0; i < nocl; i++) {
                if (clients[i].socket == sockfd) {
                    j = i;
                    while (j < nocl - 1) {
                        clients[j] = clients[j + 1];
                        j++;
                    }
                }
            }
            nocl--;
            pthread_mutex_unlock(&mutex);
            break;
        } else {
            if (buff[strlen(buff) - 1] == '\n')
                buff[strlen(buff) - 1] = '\0';
            if (buff[strlen(buff) - 1] == '\r')
                buff[strlen(buff) - 1] = '\0';
            int type = (*buff >> 4) & 15;
            switch (type) {
                case CONNECT:
                    set_connect(sockfd, buff);
                    continue;
                case SUBSCRIBE:
                    subscribe(sockfd, buff);
                    continue;
                case GET:
                    get_list(sockfd);
                    continue;
                case PUBLISH:
                    check_publish(sockfd, buff);
                    continue;
                case CREATE:
                    create_room(sockfd, buff);
                    continue;
                case SENDFILE:
                    recv_file(sockfd, buff);
                    continue;
                case DOWNFILE:
                    send_file(sockfd, buff);
                    continue;
                case ADD:
                    add_user(sockfd, buff);
                    continue;
                default:
                    break;
            }
        }
        free(buff);
    }
    fflush(stdout);
    return 0;
}

int main(int argc, char const *argv[]) {

    int listenfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
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
    create_defaul_room(rooms);
    nor++;
    while (1) {
        pthread_t tid;
        iptr = malloc(sizeof(int));
        clilen = sizeof(cliaddr);
        *iptr = accept(listenfd, (SA *) &cliaddr, &clilen);
        char s[255];
        inet_ntop(cliaddr.sin_family, (struct sockaddr *) &cliaddr, s, sizeof s);
        printf("Server get connection from %s\n", s);
        pthread_create(&tid, NULL, &handle, (void *) iptr);
    }
    return 0;
}