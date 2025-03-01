#ifndef ARGP_PARSE_H
#define ARGP_PARSE_H

#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct arguments {
    char username[256];
    char password[256];
    char brokerid[128];
    int port;
    char ca[512];
    char cert[512];
    char key[512];
    int daemonize;
};

int parse_arguments(int argc, char **argv, struct arguments *args);

void parse_error_print(int code);

#endif