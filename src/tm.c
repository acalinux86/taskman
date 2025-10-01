#include <sqlite3.h>
#include "tm.h"

const char *tm_priority_as_cstr(TM_Priority p)
{
    switch (p) {
    case TM_LOW:    return "LOW";
    case TM_MEDIUM: return "MEDIUM";
    case TM_HIGH:   return "HIGH";
    case TM_PRIORITY_COUNT:
    default:
        return NULL;
    }
}

TM_Priority tm_priority_from_cstr(const char *p_cstr)
{
    int i;
    for (i = 0; i < TM_PRIORITY_COUNT; ++i) {
        if (strcmp(tm_priority_as_cstr(i), p_cstr) == 0) break;
    }
    return (TM_Priority)i;
}

void tm_sqlite3_version(void)
{
    fprintf(stdout, "Sqlite3 Version: %s.\n", sqlite3_libversion());
}

const char *tm_query_types_as_cstr(TM_QueryType t)
{
    switch (t) {
    case TM_SELECT_ALL: return "DB_SELECT_ALL";
    case TM_SELECT_ONE: return "DB_SELECT_ONE";
    case TM_DELETE_ALL: return "DB_DELETE_ALL";
    case TM_DELETE_ONE: return "DB_DELETE_ONE";
    case TM_INSERT:     return "DB_INSERT";
    case TM_UPDATE:     return "DB_UPDATE";
    case TM_QUERY_COUNT:
    default:
        return NULL;
    }
}

TM_QueryType tm_query_type_from_cstr(const char *type_as_cstr)
{
    int i;
    for (i = 0; i < TM_QUERY_COUNT; ++i) {
        if (strcmp(tm_query_types_as_cstr(i), type_as_cstr) == 0) break;
    }
    return (TM_QueryType)i;
}

void tm_db_begin(const char *db_path, sqlite3 *db)
{
    int result = sqlite3_open(db_path, &db);
    if (result != SQLITE_OK) {
        fprintf(stdout, "[ERROR] Failed to Open a Sqlite3 Connection: %s\n", sqlite3_errmsg(db));
    } else {
        fprintf(stdout, "[INFO] Successfully Opened a Sqlite3 Connection: %p\n", (void*)db);
    }
}

char *tm_query_task(TM_Query *query, const char *table, unsigned int *ID, const TM_Task *task, TM_Priority *priority)
{
    char buffer[TM_BUFFER_LEN];
    switch(query->type) {
    case TM_SELECT_ALL: {
        snprintf(buffer, sizeof(buffer), "SELECT * FROM %s;", table);
        query->statement = (TM_QueryData *)malloc(sizeof(TM_QueryData));
        query->statement->select_all_rows = tm_strdup(buffer);
    } break;

    case TM_SELECT_ONE: {
        snprintf(buffer, sizeof(buffer), "SELECT Id, Task, Priority FROM %s WHERE Id = %d;", table, *ID);
        query->statement = (TM_QueryData *)malloc(sizeof(TM_QueryData));
        query->statement->select_row = tm_strdup(buffer);
    } break;

    case TM_DELETE_ALL: {
        snprintf(buffer, sizeof(buffer), "DROP FROM %s;", table);
        query->statement = (TM_QueryData *)malloc(sizeof(TM_QueryData));
        query->statement->delete_all_rows = tm_strdup(buffer);
    } break;

    case TM_DELETE_ONE: {
        if (priority != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE trim(Priority) LIKE '%s';", table, tm_priority_as_cstr(*priority));
        } else if (ID != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE Id = %d;", table, *ID);
        }
        query->statement = (TM_QueryData *)malloc(sizeof(TM_QueryData));
        query->statement->delete_all_rows = tm_strdup(buffer);
    } break;

    case TM_INSERT: {
        char *task_msg = tm_strdup(task->message);
        const char *priority = tm_priority_as_cstr(task->priority);
        char *boolean_str = task->done == true ? "true" : "false";
        snprintf(buffer, sizeof(buffer), "INSERT INTO %s(Task, Priority, Done) VALUES('%s','%s','%s');", table, task_msg, priority, boolean_str);
        free(task_msg);
    } break;

    case TM_UPDATE: {
        snprintf(buffer, sizeof(buffer), "UPDATE %s SET Done = 'true' WHERE Id = %d;", table, *ID);
        query->statement = (TM_QueryData *)malloc(sizeof(TM_QueryData));
        query->statement->update_row = tm_strdup(buffer);
    } break;

    default:
        fprintf(stderr, "ERROR: UnKnown Type.\n");
        return NULL;
    }

    return tm_strdup(buffer);
}


void tm_db_end(sqlite3 *db)
{
    sqlite3_close(db);
    fprintf(stdout, "[INFO] Successfully Closed a Sqlite3 Connection: %p\n", (void*)db);
}
