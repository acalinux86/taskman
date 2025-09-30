#include <sqlite3.h>

#include "task.h"

const char *priority_as_cstr(Priority p)
{
    switch (p) {
    case LOW:    return "LOW";
    case MEDIUM: return "MEDIUM";
    case HIGH:   return "HIGH";
    case PRIORITY_COUNT:
    default:
        return NULL;
    }
}

const char *query_types_as_cstr(Query_Type t)
{
    switch (t) {
    case DB_SELECT_ALL: return "DB_SELECT_ALL";
    case DB_SELECT_ONE: return "DB_SELECT_ONE";
    case DB_DELETE_ALL: return "DB_DELETE_ALL";
    case DB_DELETE_ONE: return "DB_DELETE_ONE";
    case DB_INSERT:     return "DB_INSERT";
    case DB_UPDATE:     return "DB_UPDATE";
    case QUERY_TYPE_COUNT:
    default:
        return NULL;
    }
}

Query_Type map_query_to_cstr(const char *type_as_cstr)
{
    int i;
    for (i = 0; i < QUERY_TYPE_COUNT; ++i) {
        if (strcmp(query_types_as_cstr(i), type_as_cstr) == 0) break;
    }

    return (Query_Type)i;
}

void our_sqlite3_version()
{
    fprintf(stdout, "Sqlite3 Version: %s.\n", sqlite3_libversion());
}

char *query_task(Query *query, const char *table, unsigned int *ID, const Task *task, Priority *priority)
{
    char buffer[BUFFER_LEN];
    switch(query->type) {
    case DB_SELECT_ALL:
        snprintf(buffer, sizeof(buffer), "SELECT * FROM %s;", table);
        query->statement = (Query_Data *)malloc(sizeof(Query_Data));
        query->statement->select_all_rows = strdup_custom(buffer);
        break;

    case DB_SELECT_ONE:
        snprintf(buffer, sizeof(buffer), "SELECT Id, Task, Priority FROM %s WHERE Id = %d;", table, *ID);
        query->statement = (Query_Data *)malloc(sizeof(Query_Data));
        query->statement->select_row = strdup_custom(buffer);
        break;

    case DB_DELETE_ALL:
        snprintf(buffer, sizeof(buffer), "DROP FROM %s;", table);
        query->statement = (Query_Data *)malloc(sizeof(Query_Data));
        query->statement->delete_all_rows = strdup_custom(buffer);
        break;

    case DB_DELETE_ONE:
        if (priority != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE trim(Priority) LIKE '%s';", table, priority_as_cstr(*priority));
        } else if (ID != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE Id = %d;", table, *ID);
        }
        query->statement = (Query_Data *)malloc(sizeof(Query_Data));
        query->statement->delete_all_rows = strdup_custom(buffer);
        break;

    case DB_INSERT: {
        char *task_msg = strdup_custom(task->message);
        const char *priority = priority_as_cstr(task->priority);
        char *boolean_str = task->done == true ? "true" : "false";
        snprintf(buffer, sizeof(buffer), "INSERT INTO %s(Task, Priority, Done) VALUES('%s','%s','%s');", table, task_msg, priority, boolean_str);
        free(task_msg);
    } break;

    case DB_UPDATE:
        snprintf(buffer, sizeof(buffer), "UPDATE %s SET Done = 'true' WHERE Id = %d;", table, *ID);
        query->statement = (Query_Data *)malloc(sizeof(Query_Data));
        query->statement->update_row = strdup_custom(buffer);
        break;

    default:
        fprintf(stderr, "ERROR: UnKnown Type.\n");
        return NULL;
    }

    return strdup_custom(buffer);
}
