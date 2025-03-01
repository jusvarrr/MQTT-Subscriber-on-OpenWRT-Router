#ifndef MQTT_SUBSCRIBER_H
#define MQTT_SUBSCRIBER_H

#include <syslog.h>
#include <signal.h>
#include <mosquitto.h>
#include <cJSON.h>
#include "uci_functions.h"
#include "argp_parse.h"
#include "event.h"
#include "event_handling.h"
#include "sqlite_logging.h"

struct uci_configs {
    char **topics;
    struct event_parameters **events;
};

void full_cleanup(struct mosquitto **mosq, char ***topics);

int mqtt_subscriber(struct arguments arguments, struct mosquitto **mosq, char ***topics, struct uci_configs *configs, int *stop);

#endif