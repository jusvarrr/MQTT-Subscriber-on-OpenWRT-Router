#ifndef UCI_FUNCTIONS_H
#define UCI_FUNCTIONS_H
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <uci.h>
#include "event.h"
#include "mqtt_subscriber.h"


void list_cleanup();

int uci_list_to_string_list(struct uci_list *list, char ***topics, size_t *current_size);

int get_topic_list(char ***topics);

int get_email_config(char *id, char *email_addr, char *smtp_serv, int *port, char *usr, char *psw);

int clean_event_list(struct event_parameters ***events);

int read_rules(struct event_parameters ***rules);

#endif