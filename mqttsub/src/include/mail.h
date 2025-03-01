#ifndef MAIL_H
#define MAIL_H

#include <time.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdbool.h>
#include "event.h"
#include "uci_functions.h"

int form_payload(char *sender, char **receivers, char **output_payload, char *subject, char *text, char **rec, char *fullemail);
int sendmail(struct event_parameters *rules, char *subject, char *text);
#endif
