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
    pp = pp + HEADER_LEN;
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
    pp = pp + HEADER_LEN;
    strcat(pp, name);
    strcat(pp, ",");
    strcat(pp, pwd);
    return packet;
}