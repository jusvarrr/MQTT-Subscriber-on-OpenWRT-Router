#include "event_handling.h"

int condition_check_numbers(char *operator, double number, double ref_number) {
	if (strcmp(operator, "<") != 0 && strcmp(operator, ">") != 0 && strcmp(operator, "==") != 0 && 
	strcmp(operator, "!=") != 0 && strcmp(operator, "<=") != 0 && strcmp(operator, ">=") != 0)
		return -1;

	if (strcmp(operator, "<") == 0 && number < ref_number) {
		return 1;
	} else if (strcmp(operator, ">") == 0 && number > ref_number) {
		return 1;
	} else if (strcmp(operator, "==") == 0 && number == ref_number) {
		return 1;
	} else if (strcmp(operator, "!=") == 0 && number != ref_number) {
		return 1;
	} else if (strcmp(operator, ">=") == 0 && number >= ref_number) {
		return 1;
	} else if (strcmp(operator, "<=") == 0 && number <= ref_number) {
		return 1;
	}

	return 0;
}

bool condition_check_bool(char *operator, bool bool_val, bool ref_bool) {
	if (strcmp(operator, "==") != 0 && strcmp(operator, "!=") != 0)
		return -1;

	return ((bool_val == ref_bool) ? true : false);
}

int condition_check_string(char *operator, char *string, char *ref_string) {
	if (strcmp(operator, "<") != 0 && strcmp(operator, ">") != 0 && strcmp(operator, "==") != 0 && 
	strcmp(operator, "!=") != 0 && strcmp(operator, "<=") != 0 && strcmp(operator, ">=") != 0)
		return -1;

	if (strcmp(operator, "<") == 0 && strcmp(string, ref_string) < 0) {
		return 1;
	} else if (strcmp(operator, ">") == 0 && strcmp(string, ref_string) > 0) {
		return 1;
	} else if (strcmp(operator, "==") == 0 && strcmp(string, ref_string) == 0) {
		return 1;
	} else if (strcmp(operator, "!=") == 0 && strcmp(string, ref_string) != 0) {
		return 1;
	} else if (strcmp(operator, ">=") == 0 && strcmp(string, ref_string) >= 0) {
		return 1;
	} else if (strcmp(operator, "<=") == 0 && strcmp(string, ref_string) <= 0) {
		return 1;
	}

	return 0;
}

void format_message_for_string(cJSON *current, struct event_parameters *event, const char *topic, const char *param, char *text, char *subject) {
    if (strcmp(event->comparison, ">") == 0 || strcmp(event->comparison, "<") == 0) {
        snprintf(text, 1024, "Parameter %s of topic %s has changed. It is now \"%s\" and \"%s\" is alphabetically %s than \"%s\".", 
                 param, topic, current->valuestring, current->valuestring,
                 !strcmp(event->comparison, ">") ? "higher" : "lower", event->ref_string);

    } else if (strcmp(event->comparison, ">=") == 0 || strcmp(event->comparison, "<=") == 0) {
        snprintf(text, 1024, "Parameter %s of topic %s has changed. It is now \"%s\" and \"%s\" is alphabetically %s or equal to \"%s\".", 
                 param, topic, current->valuestring, current->valuestring,
                 !strcmp(event->comparison, ">=") ? "higher" : "lower", event->ref_string);

    } else if (strcmp(event->comparison, "==") == 0 || strcmp(event->comparison, "!=") == 0) {
        snprintf(text, 1024, "Parameter %s of topic %s has changed. It is now \"%s\" and is %sequal to \"%s\".", 
                 param, topic, event->ref_string, strcmp(event->comparison, "==") ? "" : "not ", current->valuestring);
    }
}


void process_event(struct event_parameters *event, const char *topic, const char *param, cJSON *current) {
    char subject[96], text[1024];
    int ret;

    snprintf(subject, 96, "Change of %s", param);

    if (strcmp(event->type, "number") == 0 && cJSON_IsNumber(current)) {
        double value = current->valuedouble;
        double value_int = (double) current->valueint;

        syslog(LOG_DEBUG, "Number comparison: value=%f, value_int=%f, ref=%f, operator=%s",
               value, value_int, event->ref_num, event->comparison);

        if (condition_check_numbers(event->comparison, value, event->ref_num) || 
            condition_check_numbers(event->comparison, value_int, event->ref_num)) {

            snprintf(text, sizeof(text), "Parameter %s of topic %s has changed. It is now %f (%s %f).", 
                     param, topic, value, event->comparison, event->ref_num);

            if ((ret = sendmail(event, subject, text)) == CURLE_OK)
                syslog(LOG_USER | LOG_INFO, "Sending message since %f %s %f", value, event->comparison, event->ref_num);
            else syslog(LOG_USER | LOG_INFO, "Could not send mail.");
        }

    } else if (!strcmp(event->type, "bool") && cJSON_IsBool(current) && 
               !condition_check_bool(event->comparison, cJSON_IsTrue(current), event->ref_bool)) {

        snprintf(text, 1024, "Parameter %s of topic %s has changed. It is now %s and %s is expected.", 
                 param, topic, cJSON_IsTrue(current) ? "true" : "false", event->ref_bool ? "false" : "true");

        ret = sendmail(event, subject, text);
        if (ret == CURLE_OK)
            syslog(LOG_USER | LOG_INFO, "Sending message since %s is expected to be %s", param, event->ref_bool ? "false" : "true");
        else syslog(LOG_USER | LOG_INFO, "Could not send mail.");

    } else if (!strcmp(event->type, "string") && cJSON_IsString(current) && 
               condition_check_string(event->comparison, current->valuestring, event->ref_string)) {
        format_message_for_string(current, event, topic, param, text, subject);
        ret = sendmail(event, subject, text);
        if (ret == CURLE_OK)
            syslog(LOG_USER | LOG_INFO, "%s", text);
        else syslog(LOG_USER | LOG_INFO, "Could not send mail.");
    }
}

void check_events(struct event_parameters **events, const char *topic, const char *param, cJSON *current) {
    int i = 0;

    while (events[i] != NULL) {
        if (events[i]->topic == NULL) {
            syslog(LOG_USER | LOG_ERR, "Invalid event or topic at index %d", i);
            break;
        }

        if (!strcmp(topic, events[i]->topic) && !strcmp(param, events[i]->name)) {
            syslog(LOG_USER | LOG_INFO, "Event for topic %s found with matching parameter name %s", events[i]->topic, param);
            process_event(events[i], topic, param, current);
        }

        i++;
    }
}

int json_parameters_recursive_check(char *fullparam, cJSON *current, struct event_parameters **events, char *topic) {
	
    if (!cJSON_IsObject(current)) {
		char newl[64];
		strcpy(newl, fullparam);
		if (64 > (strlen(newl) + strlen(current->string) + 2)) {
			strncat(newl, current->string, strlen(current->string));
		}
		check_events(events, topic, newl, current);
	}
	
	if (current->next != NULL) {
		char newl[64];
		strcpy(newl, fullparam);
		
		if (json_parameters_recursive_check(newl, current->next, events, topic) != 0)
			return -1;
	}

	if (current->child != NULL) {
		if (64 > (strlen(fullparam) + strlen(current->string) + 2)) {
			strncat(fullparam, current->string, strlen(current->string));
			strncat(fullparam, ".", 2);
		}

		if (json_parameters_recursive_check(fullparam, current->child, events, topic) != 0)
			return -1;
	}

	return 0;
}