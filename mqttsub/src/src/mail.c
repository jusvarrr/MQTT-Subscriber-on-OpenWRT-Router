#include "mail.h"

char *payload_text = NULL;

struct upload_status {
  size_t bytes_read;
};

char *extract_domain(char *email) {
    if (email == NULL) {
        return NULL;
    }
    char *at_symbol = strchr(email, '@');
    if (at_symbol && *(at_symbol + 1)) {
        return at_symbol + 1;
    }
    return NULL;
}

void generate_message_id(char *message_id, size_t size, char *email) {
    time_t now = time(NULL);
    int random_number = rand() % 100000;
    char *domain = extract_domain(email);
    snprintf(message_id, size, "<%ld.%d@%s>", now, random_number, domain);
}

static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = &payload_text[upload_ctx->bytes_read];
 
  if(data) {
    size_t len = strlen(data);
    if(room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
 
    return len;
  }
 
  return 0;
}

int form_receivers(char **receivers, char *line, int maxlen) {
    int i = 0, len = 0;
    line[0] = '\0';
    
    if (receivers == NULL || receivers[i] == NULL )
        return -1;

    while (receivers[i] != NULL && ((len + strlen(receivers[i]) + 2) < maxlen) ) {
        len += strlen(receivers[i]) + 2;
        strcat(line, "<");
        strcat(line, receivers[i]);
        strcat(line, ">");
        i++;
        if (receivers[i] != NULL && len + 2 < maxlen) {
            strcat(line, ", ");
            len += 2;
        } else {
            break;
        } 
    }

    line[maxlen-1] = '\0';
    return 0;
}

void format_date(char *date) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(date, strlen("Mon, 29 Nov 2010 21:54:29 UTC") + 1, 
             "%a, %d %b %Y %H:%M:%S %Z", &tm);
}

int form_payload(char *sender, char **receivers, char **output_payload, char *subject, char *text, char **rec, char *fullemail) {
    int ret = 0, i = 0, estimated_size = 1;
    char message_id[128];
    char date[32];
    srand(time(NULL));

    generate_message_id(message_id, sizeof(message_id), fullemail);

    if (receivers == NULL || receivers[i] == NULL )
        return -1;

    while (receivers[i] != NULL) {
        estimated_size += strlen(receivers[i]) + 4;
        i++;
    }
    
    char *to_field = (char *)malloc(estimated_size);
    
    if (!to_field)
        return -2;

    if ((ret = form_receivers(receivers, to_field, estimated_size)) != 0) {
        free(to_field);
        return -3;
    }

    format_date(date);
    estimated_size += strlen(sender) + strlen(date) + strlen(text) + 1024;
        
    *rec = to_field;
    payload_text = (char *)malloc(estimated_size);
    if (!payload_text) {
        return -4;
    }

    int written = snprintf(payload_text, estimated_size,
             "Date: %s\r\n"
             "To: %s\r\n"
             "From: %s\r\n"
             "Cc: \r\n"
             "Message-ID: %s\r\n"
             "Subject: %s\r\n"
             "\r\n"
             "%s\r\n"
             "\r\n"
             "Check RFC 5322.\r\n",
             date, to_field, sender, message_id, subject, text);
    if (written < 0) {
        free(to_field);
        free(payload_text);
        return -5;
    }
    *output_payload = payload_text;
    return 0;
}

int sendmail(struct event_parameters *rules, char *subject, char *text) {
    char *rec = NULL;
    char curl_addr[140] = {0}, smtp_serv[128] = {0}, usr[128] = {0}, psw[128] = {0}, sender[128] = {0};
    int port = 25;


    int ret = get_email_config(rules->email, sender, smtp_serv, &port, usr, psw);
    if (ret != 0){
        syslog(LOG_USER | LOG_ERR, "parsing email configs failed");
        return -1;
    }

    ret = form_payload(rules->email, rules->recipients, &payload_text, subject, text, &rec, sender);
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx = { 0 };
    curl = curl_easy_init();

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        snprintf(curl_addr, sizeof(curl_addr), "smtps://%s:%d", smtp_serv, port);
        
        curl_easy_setopt(curl, CURLOPT_URL, curl_addr);
        curl_easy_setopt(curl, CURLOPT_USERNAME, usr);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, psw);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sender);

        recipients = curl_slist_append(recipients, rec);
        if (!recipients) {
            syslog(LOG_USER | LOG_ERR, "curl_slist_append() failed");
            curl_easy_cleanup(curl);
            free(payload_text);
            return -2;
        }

        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    
        res = curl_easy_perform(curl);
    
        if(res != CURLE_OK)
        syslog( LOG_USER | LOG_ERR, "curl_easy_perform() failed: %s",
                curl_easy_strerror(res));
    
        curl_slist_free_all(recipients);
    
        curl_easy_cleanup(curl);
        if (payload_text != NULL) free(payload_text);
        if (rec != NULL) free(rec);
    }

    return res;
}