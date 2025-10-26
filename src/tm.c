#include <sqlite3.h>
#include <errno.h>

#include "tm.h"

sqlite3 *db = NULL;
const char *table = "Tasks";

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

void tm_db_begin(const char *db_path)
{
    int result = sqlite3_open(db_path, &db);
    if (result != SQLITE_OK) {
        fprintf(stderr, "[ERROR] Failed to Open a Sqlite3 Connection: %s\n", sqlite3_errmsg(db));
    } else {
        fprintf(stdout, "[INFO] Successfully Opened a Sqlite3 Connection: %p\n", (void*)db);
    }
}

char *tm_db_query_task(TM_QueryType type, const char *table, const int *ID, const TM_Task *task, TM_Priority *priority)
{
    char buffer[TM_BUFFER_LEN];
    switch(type) {
    case TM_CREATE_TABLE: {
        snprintf(buffer, sizeof(buffer), "CREATE TABLE IF NOT EXISTS %s(Id INTEGER PRIMARY KEY AUTOINCREMENT, Task TEXT NOT NULL, Priority TEXT NOT NULL, Done TEXT NOT NULL);", table);
    } break;

    case TM_SELECT_ALL: {
        snprintf(buffer, sizeof(buffer), "SELECT * FROM %s;", table);
    } break;

    case TM_SELECT_ONE: {
        snprintf(buffer, sizeof(buffer), "SELECT Id, Task, Priority FROM %s WHERE Id = %d;", table, *ID);
    } break;

    case TM_DELETE_ALL: {
        snprintf(buffer, sizeof(buffer), "DROP FROM %s;", table);
    } break;

    case TM_DELETE_ONE: {
        if (priority != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE trim(Priority) LIKE '%s';", table, tm_priority_as_cstr(*priority));
        } else if (ID != NULL) {
            snprintf(buffer, sizeof(buffer), "DELETE FROM %s WHERE Id = %d;", table, *ID);
        }
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
    } break;

    default:
        fprintf(stderr, "ERROR: UnKnown Type.\n");
        return NULL;
    }
    return tm_strdup(buffer);
}

void tm_db_end()
{
    int result = sqlite3_close(db);
    assert(result != SQLITE_BUSY);
    if (result != SQLITE_OK) {
        fprintf(stderr, "[ERROR] Failed to Close Sqlite3 Connection: %s\n", sqlite3_errmsg(db));
    } else {
        fprintf(stdout, "[INFO] Successfully Closed a Sqlite3 Connection: %p\n", (void*)db);
    }
}

bool tm_parse_integer(const char *cstr_i, int *number)
{
    char *endptr, *str;
    int base = 10; // parse decimal

    str = (char *)cstr_i;
    errno = 0;
    *number = strtoul(str, &endptr, base);
    if (errno == ERANGE) {
        perror("strtoul");
        return false;
    }

    if (endptr == str) {
        //fprintf(stderr, "No digits were found\n");
        return false;
    }
    return true;
}

int callback(void *not_used, int argc, char **argv, char **az_colname) {
    (void) not_used;
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", az_colname[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

char *tm_db_list_tasks()
{
    TM_QueryType query = TM_SELECT_ALL;
    char *buffer = tm_db_query_task(query, table, NULL, NULL, NULL);
    assert(buffer != NULL);
    return buffer;
}

char *tm_db_insert_task(const TM_Task *task)
{
    TM_QueryType query = TM_INSERT;
    char *buffer = tm_db_query_task(query, table, NULL, task, NULL);
    assert(buffer != NULL);
    return buffer;
}

char *tm_db_create_table()
{
    TM_QueryType query = TM_CREATE_TABLE;
    char *buffer = tm_db_query_task(query, table, NULL, NULL, NULL);
    assert(buffer != NULL);
    return buffer;
}

char *tm_db_delete_task(const int *id)
{
    TM_QueryType query = TM_DELETE_ONE;
    char *buffer = tm_db_query_task(query, table, id, NULL, NULL);
    assert(buffer != NULL);
    return buffer;
}

char *tm_db_update_task(const int *id)
{
    TM_QueryType query = TM_UPDATE;
    char *buffer = tm_db_query_task(query, table, id, NULL, NULL);
    assert(buffer != NULL);
    return buffer;
}

const char *tm_shift_args(int *argc, char ***argv)
{
    const char *result = **argv;
    (*argc)--;
    (*argv)++;
    return result;
}

void tm_usage(const char *program)
{
    FILE *out = stderr;
    fprintf(out, "\nTask Manager - Manage priority-based tasks\n\n");
    fprintf(out, "USAGE: %s [OPTIONS]\n\n", program);
    fprintf(out, "OPTIONS:\n");
    fprintf(out, "  --task, -t <message>    Specify task description (required)\n");
    fprintf(out, "  --priority, -p <value>  Set task priority (required)\n");
    fprintf(out, "                          Values: 0|LOW, 1|MEDIUM, 2|HIGH\n");
    fprintf(out, "  --delete, -d <id>       Specify task id to be deleted (required)\n");
    fprintf(out, "  --finish, -f <id>       Specify task id to be marked done (required)\n");
    fprintf(out, "  --list, -l              List the Available Tasks\n");
    fprintf(out, "  --version, -v           Print out the database details\n");
    fprintf(out, "  --help, -h              Show this help message\n\n");
    fprintf(out, "EXAMPLES:\n");
    fprintf(out, "  %s --task \"Finish project\" --priority HIGH\n", program);
    fprintf(out, "  %s --delete 1 \n", program);
    fprintf(out, "  %s --finish 1 \n", program);
}

bool tm_parse_cli(TM_Tasks *tasks, TM_String *buffers, const char *program, int *argc, char ***argv)
{
    while (*argc > 0) {
        const char *current_flag = tm_shift_args(argc, argv);
        if (strcmp(current_flag, "--version") == 0 || strcmp(current_flag, "-v") == 0) {
            tm_sqlite3_version();
        } else if (strcmp(current_flag, "--list") == 0 || strcmp(current_flag, "-l") == 0) {
            char *query = tm_strdup((char*)tm_db_list_tasks());
            array_append(buffers, query);
        } else if (strcmp(current_flag, "--delete") == 0 || strcmp(current_flag, "-d") == 0) {
            if (*argc > 0) {
                const int id = atoi((char*)tm_shift_args(argc, argv));
                char *query = tm_db_delete_task(&id);
                array_append(buffers, query);
            } else {
                tm_usage(program);
                fprintf(stderr, "\nERROR: Cannot delete task, No id provided after `%s` flag.\n", current_flag);
                return false;
            }
        } else if (strcmp(current_flag, "--finish") == 0 || strcmp(current_flag, "-f") == 0) {
            if (*argc > 0) {
                const int id = atoi((char*)tm_shift_args(argc, argv));
                char *query = tm_db_update_task(&id);
                array_append(buffers, query);
            } else {
                tm_usage(program);
                fprintf(stderr, "\nERROR: Cannot update task, No id provided after `%s` flag.\n", current_flag);
                return false;
            }
        } else if (strcmp(current_flag, "--task") == 0 || strcmp(current_flag, "-t") == 0) {
            // When we see the task flag,  we r starting a new task
            TM_Task task = {0};
            if (*argc > 0) {
                const char *message = tm_shift_args(argc, argv);
                if (strcmp(message, "--priority") == 0 || strcmp(message, "--p") == 0) {
                    tm_usage(program);
                    fprintf(stderr, "\nERROR: NO Task Provided after `%s` flag.\n",current_flag);
                    return false;
                } else {
                    task.message = (char*)message;//tm_strdup((char*) message); // register task message
                }
                if (*argc > 0) {
                    const char *priority = tm_shift_args(argc, argv);
                    if (strcmp(priority, "--priority") == 0 || strcmp(priority, "-p") == 0) {
                        if (*argc > 0) {
                            const char *priority_level = tm_shift_args(argc, argv);
                            int number;
                            bool is_number = tm_parse_integer(priority_level, &number);
                            if (is_number) {
                                if (number < 3 && number >= 0) {
                                    task.priority = number; // register priority if given as integer
                                } else {
                                    tm_usage(program);
                                    fprintf(stderr, "\nERROR: UnKnown Priority Level `%d` Provided.\n", number);
                                    return false;
                                }
                            } else {
                                // if its not a number then its string
                                task.priority = tm_priority_from_cstr(priority_level); // register priority if given as string
                            }
                        } else {
                            tm_usage(program);
                            fprintf(stderr, "\nERROR: No Priority Level Provided.\n");
                            return false;
                        }
                    }
                } else {
                    fprintf(stderr, "\nWARN: No Priority Provided for task `%s`. Defaulting to LOW priority\n", message);
                    array_append(tasks, task);
                    // incase of more than one task dont return early // return true; // Finished parsing , got task and priority return early for now,
                }
            } else {
                tm_usage(program);
                fprintf(stderr, "\nERROR: No Task Provided.\n");
                return false;
            }
            array_append(tasks, task); // Append and continue parsing
        } else {
            tm_usage(program);
            if (strcmp(current_flag, "--priority") == 0 || strcmp(current_flag, "--p") == 0) {
                fprintf(stderr, "ERROR: Provide task to set the Priority.\n");
            } else {
                fprintf(stderr, "ERROR: UnKnown Flag `%s` Provided.\n", current_flag);
            }
            return false;
        }
    }
    return true;
}

bool tm_db_register_tasks(const TM_Tasks *tasks, TM_String *string)
{
    if (!tasks || tasks->count == 0) return false;
    for (uint32_t i = 0; i < tasks->count; ++i) {
        char *str =  (char*)tm_db_insert_task(&tasks->items[i]);
        array_append(string, str);
    }
    return true;
}

void tm_debug_tasks(const TM_Tasks *tasks)
{
    for (uint32_t i = 0; i < tasks->count; ++i) {
        TM_Task *task = &tasks->items[i];
        fprintf(stdout, "[INFO] ID: %d, Task: `%s` Priority: `%s` Done: %s.\n", task->id, task->message, tm_priority_as_cstr(task->priority), task->done == 0 ? "false" : "true");
    }
}

bool tm_db_execute_query(const char *query_buf)
{
    char *err_msg = NULL;
    int result = sqlite3_exec(db, query_buf, callback, 0, &err_msg);
    if (result != SQLITE_OK) {
        if (err_msg) {
            fprintf(stderr, "[ERROR] %s\n", err_msg);
            sqlite3_free(err_msg);
        } else {
            fprintf(stderr, "[ERROR] Unknown SQL Error\n");
        }
        return false;
    } else {
        fprintf(stdout, "[INFO] Successfully Queried\n");
    }
    return true;
}
