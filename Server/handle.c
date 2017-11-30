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

void create_account(struct Accounts a) {
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
}

struct Accounts decode_packet_connect(char *buff) {
    char *p = buff;
    p = p + 1;
    struct Accounts a;
    strcpy(a.name, strtok(p, ","));
    a.name[strlen(a.name) - 1] = '\0';
    strcpy(a.pwd, strtok(NULL, ","));
    a.pwd[strlen(a.pwd) - 1] = '\0';
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
        return FALSE;
    else
        return TRUE;
}