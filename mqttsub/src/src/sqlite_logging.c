#include "sqlite_logging.h"

sqlite3 *db;

int db_open() {
    int rc = 0;

    rc = sqlite3_open("/log/topics.db", &db);
    if (rc != SQLITE_OK) {
        return rc;
    } else {
        syslog(LOG_USER | LOG_INFO, "Database opened successfully!");
        return rc;
    }
}

int table_exists(char *table_name) {
    int exists = 0;
    char sql[256];
    sqlite3_stmt *stmt;

    snprintf(sql, sizeof(sql), "SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", 
        table_name);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = 1;
        }
    }

    sqlite3_finalize(stmt);
    return exists;
}

int create_table(char *table_name) {
    char sql[512];
    snprintf(sql, sizeof(sql), 
        "CREATE TABLE IF NOT EXISTS %s ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "topic TEXT NOT NULL, "
        "payload TEXT NOT NULL, "
        "sent_at DATETIME DEFAULT CURRENT_TIMESTAMP);", 
        table_name);

    int rc = sqlite3_exec(db, sql, NULL, NULL, NULL);

    return rc;
}

int db_log_message(char *topic, char *payload) {
    int rc = 0, exists = 0;

    sqlite3_stmt *stmt = NULL;
    char sql[1024];

    if (!table_exists("mqtt_messages")) {
        if ((rc = create_table("mqtt_messages")) != SQLITE_OK)
            return rc;
    }

    strcpy(sql, "SELECT id FROM mqtt_messages WHERE topic = ?;");
    
    if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
        return rc;

    if ((rc = sqlite3_bind_text(stmt, 1, topic, -1, NULL)) != SQLITE_OK)
        return rc;

    if (sqlite3_step(stmt) == SQLITE_ROW)
        exists = sqlite3_column_int(stmt, 0);
    
    if ((rc = sqlite3_finalize(stmt)) != SQLITE_OK) 
        return rc;

    if (exists) {
        strcpy(sql, "UPDATE mqtt_messages SET payload = ?, sent_at = CURRENT_TIMESTAMP WHERE id = ?;");
            
        if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
            return rc;

        if ((rc = sqlite3_bind_text(stmt, 1, payload, -1, NULL)) != SQLITE_OK) 
            return rc;

        if ((rc = sqlite3_bind_int(stmt, 2, exists)) != SQLITE_OK)
            return rc;

    } else {
        strcpy(sql, "INSERT INTO mqtt_messages (topic, payload) VALUES (?,?)");
        if ((rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL)) != SQLITE_OK)
            return rc;

        if ((rc = sqlite3_bind_text(stmt, 1, topic, -1, NULL)) != SQLITE_OK)
            return rc;

        if ((rc = sqlite3_bind_text(stmt, 2, payload, -1, NULL)) != SQLITE_OK) 
            return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        syslog(LOG_USER | LOG_ERR, "SQL execution failed: %s", sqlite3_errmsg(db));
    }

    rc = sqlite3_finalize(stmt);


    return rc;
}

int db_close() {
    sqlite3_close(db);
}