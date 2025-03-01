#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cJSON.h"
#include "event.h"
#include "mail.h"

int json_parameters_recursive_check(char *fullparam, cJSON *current, struct event_parameters **events, char *topic);

#endif