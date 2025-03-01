#ifndef SQLITE_LOGGING_H
#define SQLITE_LOGGING_H

#include <stdio.h>
#include <sqlite3.h> 
#include <syslog.h>
#include <string.h>

int db_open();
int db_log_message(char *topic, char *payload);
int db_close();

#endif