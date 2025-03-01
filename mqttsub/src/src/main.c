#include <syslog.h>
#include <signal.h>
#include "uci_functions.h"
#include "argp_parse.h"
#include "mqtt_subscriber.h"
#include "mail.h"
#include "daemon.h"
#include "sqlite_logging.h"

int stop = 0;

void sig_handler(int signum){
    stop = 1;
}

void set_signal_handlers(){
    signal(SIGINT,sig_handler);
    signal(SIGHUP,sig_handler);
    signal(SIGQUIT,sig_handler);
    signal(SIGILL,sig_handler);
    signal(SIGTRAP,sig_handler);
    signal(SIGABRT,sig_handler);
    signal(SIGFPE,sig_handler);
    signal(SIGTERM,sig_handler);
}

int main(int argc, char **argv)
{
    int ret = 0;
    
    set_signal_handlers();
    openlog("mqtt_sub", LOG_PID, LOG_USER);
    struct arguments arguments;
    int parse_result = parse_arguments(argc, argv, &arguments);
    if (parse_result != 0) {
        parse_error_print(parse_result);
        closelog();
        return 1;
    }

    if (arguments.daemonize == 1) {
        syslog(LOG_USER | LOG_INFO, "MQTT Subscriber Daemon Starting...");
        ret = become_daemon(0);
        if(ret)
        {
            syslog(LOG_USER | LOG_ERR, "error starting");
            closelog();

            return EXIT_FAILURE;
        }
    }

    syslog(LOG_USER | LOG_INFO, "MQTT Subscriber Running!");

	char **topics = NULL;
	struct mosquitto *mosq = NULL;
	struct uci_configs configs = {0};

	ret = mqtt_subscriber(arguments, &mosq, &topics, &configs, &stop);
    if (ret != 0)
        stop = 1;
    else
        db_open();

	while(!stop) { 
        ret = mosquitto_loop(mosq,  -1, 1);
        if (ret != 0) {
            while(mosquitto_reconnect(mosq) != 0 && !stop) {
                syslog(LOG_USER | LOG_INFO, "Reconnecting...");
                sleep(5);
            }
        }
        sleep(1); 
    }

    syslog(LOG_USER | LOG_INFO, "MQTT Subscriber Stopped.");
    clean_event_list(&(configs.events));
    full_cleanup(&mosq, &topics);
    
    db_close();
    closelog();
	
    return 0;
}