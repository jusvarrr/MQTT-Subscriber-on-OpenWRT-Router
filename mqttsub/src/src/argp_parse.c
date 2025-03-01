#include "argp_parse.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const char *argp_program_version = "argp-ex3 1.0";
const char *argp_program_bug_address = "<bug-gnu-utils@gnu.org>";

static char doc[] =
    "Argp example #3 -- a program with options and arguments using argp for parsing device";

static char args_doc[] = "[OPTIONS]";

static struct argp_option options[] = {
    {"username", 'u', "USER", 0, "Specify MQTT Broker Username"},
    {"password", 'P', "PASS", 0, "Specify MQTT Broker User Password"},
    {"broker_id", 'b', "BROKERID", 0, "Specify MQTT Broker ID (Required)"},
    {"port", 'p', "PORT", 0, "Specify MQTT Broker Port (Default: 1883)"},
    {"cafile", 'C', "CA", 0, "Specify CA File"},
    {"cert", 'R', "CERT", 0, "Specify Certificate File"},
    {"key", 'K', "KEY", 0, "Specify Key"},
    {"daemonize", 'D', 0, 0, "Run as daemon"},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
    case 'u':
        strncpy(arguments->username, arg, sizeof(arguments->username) - 1);
        arguments->username[sizeof(arguments->username) - 1] = '\0';
        break;
    case 'P':
        strncpy(arguments->password, arg, sizeof(arguments->password) - 1);
        arguments->password[sizeof(arguments->password) - 1] = '\0';
        break;
    case 'b':
        strncpy(arguments->brokerid, arg, sizeof(arguments->brokerid) - 1);
        arguments->brokerid[sizeof(arguments->brokerid) - 1] = '\0';
        break;
    case 'p':
        arguments->port = atoi(arg);
        break;
    case 'C':
        strncpy(arguments->ca, arg, sizeof(arguments->ca) - 1);
        arguments->ca[sizeof(arguments->ca) - 1] = '\0';
        break;
    case 'R':
        strncpy(arguments->cert, arg, sizeof(arguments->cert) - 1);
        arguments->cert[sizeof(arguments->cert) - 1] = '\0';
        break;
    case 'K':
        strncpy(arguments->key, arg, sizeof(arguments->key) - 1);
        arguments->key[sizeof(arguments->key) - 1] = '\0';
        break;
    case 'D':
        arguments->daemonize = 1;
        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

void parse_error_print(int code) {
    switch(code) {
        case -1:
            printf("No arguments provided. Use --help for usage information.\n");
            break;
        case -2:
            printf("Error parsing arguments.\n");
            break;
        case -3:
            printf("Broker ID is required.\n");
            break;
        case -4:
            printf("Provide all certificates (CA, CERT, and KEY) for TLS encryption.\n");
            break;
        case -5:
            printf("Provide both username and password for authentication.\n");
            break;
        default:
            printf("Unknown error code: %d\n", code);
            break;
    }
}


int parse_arguments(int argc, char **argv, struct arguments *args) {
    if (args == NULL)
        return -1;

    memset(args, 0, sizeof(*args));
    args->port = 1883;
    args->daemonize = 0;

    if (argc <= 1)
        return -1;
    if (argp_parse(&argp, argc, argv, ARGP_NO_EXIT, 0, args) != 0)
        return -2;

    if (strlen(args->brokerid) == 0)
        return -3;

    if ((strlen(args->ca) != 0 || strlen(args->cert) != 0 || strlen(args->key) != 0) &&
        (strlen(args->ca) == 0 || strlen(args->cert) == 0 || strlen(args->key) == 0))
        return -4;

    if ((strlen(args->username) != 0 || strlen(args->password) != 0) &&
        (strlen(args->username) == 0 || strlen(args->password) == 0))
        return -5;

    return 0;
}