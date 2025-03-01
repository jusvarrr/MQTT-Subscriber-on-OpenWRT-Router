#include "uci_functions.h"


void list_cleanup(char ***list) {
	if (list != NULL && *list != NULL) {
		size_t j = 0;
		while ((*list)[j] != NULL) {
			free((*list)[j]);
			j++;
		}
		free(*list);
		list = NULL;
	}
}

int uci_list_to_string_list(struct uci_list *list, char ***topics, size_t *current_size)
{
    struct uci_element *i;

    uci_foreach_element(list, i)
    {
        size_t element_length = strlen(i->name);

		char *name = malloc(element_length + 1);
		if (name == NULL)
			continue;

		strcpy(name, i->name);

		char **temp = realloc(*topics, (*current_size + 2) * sizeof(char *));
		if (temp == NULL) {
            free(name);
			return 1;
		}

		*topics = temp;
        
        (*topics)[(*current_size)++] = name;
        (*topics)[*current_size] = NULL;
    }

    return 0;
}

int uci_parse_event(struct uci_section *section, struct event_parameters *event)
{
    struct uci_element *i;
    size_t current_size = 0;
    char **recipients = NULL;
    size_t current_recipients_size = 0;
    struct event_parameters rule = {0};

    memset(event, 0, sizeof(struct event_parameters));
    
    uci_foreach_element(&section->options, i)
    {
        struct uci_option *option = uci_to_option(i);
        int r = 0;

        if (option == NULL)
            continue;


        if (event == NULL)
            return -1;

        if (strcmp(option->e.name, "topic") == 0 && option->v.string != NULL){
            strncpy(rule.topic, option->v.string, 255);
            rule.topic[255] = '\0';
        } else if (strcmp(option->e.name, "param_name") == 0 && option->v.string != NULL) {
            strncpy(rule.name, option->v.string, 63);
            rule.name[63] = '\0';
        } else if (strcmp(option->e.name, "param_type") == 0 && option->v.string != NULL) {
            strncpy(rule.type, option->v.string, 15);
            rule.type[15] = '\0';
        } else if (strcmp(option->e.name, "operator") == 0 && option->v.string != NULL) {
            strncpy(rule.comparison, option->v.string, 2);
            rule.comparison[2] = '\0';
        } else if (strcmp(option->e.name, "ref_value") == 0 && option->v.string != NULL && strcmp(rule.type , "number") == 0) {
            rule.ref_num = atof(option->v.string);
        } else if (strcmp(option->e.name, "ref_value") == 0 && option->v.string != NULL && strcmp(rule.type, "string") == 0) {
            strncpy(rule.ref_string, option->v.string, 255);
            rule.ref_string[255] = '\0';
        } else if (strcmp(option->e.name, "ref_value") == 0 && option->v.string != NULL && strcmp(rule.type, "bool") == 0) {
            rule.ref_bool = (strcmp(option->v.string, "true")) ? true : false;
        } else if (strcmp(option->e.name, "email_acc") == 0 && option->v.string != NULL) {
            strncpy(rule.email, option->v.string, 255);
            rule.email[255] = '\0';
        } else if (strcmp(option->e.name, "recipients") == 0 && option->v.string != NULL) {
            r = uci_list_to_string_list(&option->v.list, &recipients, &current_recipients_size);
            if (r == 0) {
                rule.recipients = recipients;
            }
        } else {
            return -1;
        }
        
    }

    if (strlen(rule.topic) == 0 || strlen(rule.name) == 0 || strlen(rule.type) == 0 ||
    strlen(rule.comparison) == 0 || (strcmp(rule.type, "string") == 0 && strlen(rule.ref_string) == 0) || 
    (strcmp(rule.type, "number") == 0 && rule.ref_num == 0) || 
    strlen(rule.email) == 0 || rule.recipients == NULL) 
    {
        return -2;
    }

    *event = rule;

    return 0;
}

int clean_event_list(struct event_parameters ***events) {
    int i = 0;
    if (*events == NULL) {
        return -1;
    }
    while ((*events)[i] != NULL && *events != NULL) {

        if ((*events)[i]->recipients != NULL) {
            
            int j = 0;
            while ((*events)[i]->recipients[j] != NULL) {
                free((*events)[i]->recipients[j]);
                j++;
            }
            free((*events)[i]->recipients);
            free((*events)[i]);

            i++;
        }
    }

    free(*events);
    events = NULL;
    return 0;
}

int event_add_to_list(struct event_parameters ***events, ssize_t *count, struct uci_section *section) {

    struct event_parameters *event = malloc(sizeof(struct event_parameters));
    memset(event, 0, sizeof(struct event_parameters));

    if (uci_parse_event(section, event) != 0) {
        free(event);
        return -1;
    }
    struct event_parameters **tmp = realloc(*events, sizeof(struct event_parameters*) * (*count + 2));
    if (tmp == NULL) {
        free(event);
        return -2;
    }

    *events = tmp;
    (*events)[(*count)++] = event;
    (*events)[*count] = NULL;

    return 0;
}

int get_email_config(char *id, char *email_addr, char *smtp_serv, int *port, char *usr, char *psw) {
    struct uci_context *context = uci_alloc_context();
    struct uci_package *package;

    if (uci_load(context, "user_groups", &package) != UCI_OK) {
        uci_perror(context, "uci_load()");
        uci_free_context(context);
        return -1;
    }

    struct uci_element *i;
    uci_foreach_element(&package->sections, i) {
        struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;
        char *section_name = section->e.name;

        if (section_type != NULL && section_name != NULL &&
            (strcmp(section_type, "email") == 0) && (strcmp(section_name, id) == 0)) {

            uci_foreach_element(&section->options, i) {
                struct uci_option *option = uci_to_option(i);

                if (option == NULL) return -2;

                if (strcmp(option->e.name, "smtp_ip") == 0 && option->v.string != NULL) {
                    strncpy(smtp_serv, option->v.string, 128);
                    smtp_serv[127] = '\0';
                } else if (strcmp(option->e.name, "username") == 0 && option->v.string != NULL) {
                    strncpy(usr, option->v.string, 128);
                    usr[127] = '\0';
                } else if (strcmp(option->e.name, "password") == 0 && option->v.string != NULL) {
                    strncpy(psw, option->v.string, 128);
                    psw[127] = '\0';
                } else if (strcmp(option->e.name, "senderemail") == 0 && option->v.string != NULL) {
                    strncpy(email_addr, option->v.string, 128);
                    email_addr[127] = '\0';
                } else if (strcmp(option->e.name, "smtp_port") == 0 && option->v.string != NULL) {
                    *port = atoi(option->v.string); 
                }
            }

            break;
        }
    }

    uci_free_context(context);
    return 0;
}

int get_topic_list(char ***topics) {
	size_t current_size = 0;
	const char *config_name = "subscriber";

    struct uci_context *context = uci_alloc_context();
    struct uci_package *package;

    if (uci_load(context, config_name, &package) != UCI_OK)
    {
        uci_perror(context, "uci_load()");
        uci_free_context(context);
        return 1;
    }

    struct uci_element *j;
    struct uci_section *section = uci_lookup_section(context, package, "topics");;
    char *section_name = section->e.name;

    if (strcmp(section_name, "topics") == 0) {

        uci_foreach_element(&section->options, j)
        {
            struct uci_option *option = uci_to_option(j);
            char *option_name = option->e.name;

            if (uci_list_to_string_list(&option->v.list, topics, &current_size) != 0)
                return 2;
        }
    }

    uci_free_context(context);
    return 0;
}

int read_rules(struct event_parameters ***rules) {
    const char *config_name = "subscriber";
    
    struct uci_context *context = uci_alloc_context();
    struct uci_package *package;

    if (uci_load(context, config_name, &package) != UCI_OK) {
        uci_perror(context, "uci_load()");
        uci_free_context(context);
        return 1;
    }

    struct uci_element *i;
    ssize_t rule_count = 0;

    uci_foreach_element(&package->sections, i) {
        struct uci_section *section = uci_to_section(i);
        char *section_type = section->type;

        if (strcmp(section_type, "rule") == 0) {
            if (event_add_to_list(rules, &rule_count, section) != 0)
                syslog(LOG_USER | LOG_ERR, "Could not read event properly");
        }
    }

    uci_free_context(context);
    return 0;
}