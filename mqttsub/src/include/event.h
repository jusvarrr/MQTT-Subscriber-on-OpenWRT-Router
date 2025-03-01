#ifndef EVENT_RULES_H
#define EVENT_RULES_H

struct event_parameters {
    char topic[256];
    char name[64];
    char type[16];
    char comparison[3];
    char ref_string[256];
    double ref_num;
    bool ref_bool;
    char email[256];
    char **recipients;
};

#endif