#include "mqtt_subscriber.h"

void full_cleanup(struct mosquitto **mosq, char ***topics) {
	if (mosq && *mosq) {
		mosquitto_destroy(*mosq);
		*mosq = NULL;
	}
    mosquitto_lib_cleanup();
	list_cleanup(topics);
}

int read_data_from_payload(cJSON **current, cJSON *json) {
	cJSON *data = cJSON_GetObjectItem(json, "data");
	*current = NULL;

	if (data && data->child != NULL)
		*current = data->child;
	else {
    	cJSON_Delete(json);
		return -1;
	}
    
	return 0;
}

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc, i = 0;
	struct uci_configs *configs = (struct uci_configs *)obj;
	char **topic_list = configs->topics;

	syslog(LOG_USER | LOG_INFO, "Callback: on_connect: %s", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		mosquitto_disconnect(mosq);
	}

	while(topic_list[i] != NULL && topic_list[i] != NULL){
		rc = mosquitto_subscribe(mosq, NULL, topic_list[i++], 1);
		if(rc != MOSQ_ERR_SUCCESS){
			syslog(LOG_USER | LOG_ERR, "Error subscribing: %s", mosquitto_strerror(rc));
			mosquitto_disconnect(mosq);
		}
	}
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	int i;
	bool have_subscription = false;

	for(i=0; i<qos_count; i++){
		syslog(LOG_USER | LOG_INFO, "Callback: on_subscribe: %d:granted qos = %d", i, granted_qos[i]);
		if(granted_qos[i] <= 2){
			have_subscription = true;
		}
	}
	if(have_subscription == false){
		syslog(LOG_USER | LOG_ERR, "Error: All subscriptions rejected.");
		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	int i = 0, rc = 0;
   	char fullparam[64] = "";
	struct uci_configs *configs = (struct uci_configs *)obj;
	char **topics = configs->topics;
	struct event_parameters **events = configs->events;
	cJSON *current = NULL;

	if ((rc = db_log_message(msg->topic, msg->payload)) != SQLITE_OK)
		syslog(LOG_USER | LOG_ERR, "MQTT message has not been logged to the database. Code: %d", rc);
	else
		syslog(LOG_USER | LOG_INFO, "MQTT message has successfully been logged to the database");
	
	syslog(LOG_USER | LOG_INFO, "Payload on topic \"%s\": %s", msg->topic, (char *)msg->payload);

	char *string = (char *)msg->payload;
    cJSON *json = cJSON_Parse(string);
	if (json == NULL) {
		syslog(LOG_USER | LOG_ERR, "Could not parse JSON.");
		return;
	}
	
	if (read_data_from_payload(&current, json) != 0) {
		syslog(LOG_USER | LOG_ERR, "No data in the payload");
		return;
	}

	if (events != NULL && events[0] != NULL)
   		json_parameters_recursive_check(fullparam, current, events, msg->topic);

    cJSON_Delete(json);

}

int mqtt_subscriber(struct arguments arguments, struct mosquitto **mosq, char ***topics, struct uci_configs *configs, int *stop) {

    int rc = 0;
	int i = 0;
	int ret = 0;

    if ((rc = get_topic_list(topics)) != 0 || *topics == NULL || (*topics)[0] == NULL){ 
		syslog(LOG_USER | LOG_ERR, "No Topics to subscribe were found!");
		list_cleanup(topics);
		return -1;
	}

	struct event_parameters **rules = NULL;
    if (read_rules(&rules) != 0)
        syslog(LOG_USER | LOG_ERR, "Error reading rules");
	else if (rules != NULL && rules[0] != 0)
		configs->events = rules;

	while(*topics != NULL && (*topics)[i] != NULL)
		syslog(LOG_USER | LOG_INFO, "[topic] %s", (*topics)[i++]);

	configs->topics = *topics;

    mosquitto_lib_init();

	*mosq = mosquitto_new(NULL, true, configs);
	if(*mosq == NULL){
		syslog(LOG_USER | LOG_ERR, "Error: Out of memory.");
		*stop = 1;
		return -3;
	}

	if (arguments.username != NULL)
    	ret = mosquitto_username_pw_set(*mosq, arguments.username, arguments.username);

	if (arguments.ca != NULL)
		mosquitto_tls_set(*mosq, arguments.ca, NULL, arguments.cert, arguments.key, NULL);

	mosquitto_connect_callback_set(*mosq, on_connect);
	mosquitto_subscribe_callback_set(*mosq, on_subscribe);
	
	mosquitto_message_callback_set(*mosq, on_message);

	
	i = 0;
	while((rc = mosquitto_connect(*mosq, arguments.brokerid, arguments.port, 30)) != MOSQ_ERR_SUCCESS 
	&& !(*stop) && i < 3) {
		syslog(LOG_USER | LOG_INFO, "Connecting...");
		i++;
		sleep(5);
	}
	if (rc != MOSQ_ERR_SUCCESS) {
		*stop = 1;
		syslog(LOG_USER | LOG_ERR, "Error Connecting: %s", mosquitto_strerror(rc));
		return -4;
	}
	
	return 0;
}