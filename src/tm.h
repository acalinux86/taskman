#ifndef TM_H_
#define TM_H_

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sqlite3.h>

#include "./array.h"

#define TM_INITIAL_CAP  10
#define TM_BUFFER_LEN   1024
#define TM_PRIORITY_LEN 10

typedef enum tm_priority {
    TM_LOW,
    TM_MEDIUM,
    TM_HIGH,
    TM_PRIORITY_COUNT,
} TM_Priority;

const char *tm_priority_as_cstr(TM_Priority p);
TM_Priority tm_priority_from_cstr(const char *p_cstr);

typedef struct tm_task {
    char *message;
    int id;
    TM_Priority priority;
    bool done;
} TM_Task;

typedef ARRAY(TM_Task) TM_Tasks; // Incase the user provides more than 1 task through cli

// Database Functions
typedef enum tm_query_type {
    TM_CREATE_TABLE,
    TM_SELECT_ALL,
    TM_SELECT_ONE,

    TM_DELETE_ALL,
    TM_DELETE_ONE,

    TM_INSERT,
    TM_UPDATE,

    TM_QUERY_COUNT,
} TM_QueryType;

typedef ARRAY(char *) TM_String;

void tm_sqlite3_version(void);

const char *tm_query_types_as_cstr(TM_QueryType t);
TM_QueryType tm_query_type_from_cstr(const char *type_as_cstr);

void tm_db_begin(const char *db_path);
char *tm_db_query_task(TM_QueryType type, const char *table, const int *ID, const TM_Task *task, TM_Priority *priority);
void tm_db_end();

bool tm_parse_cli(TM_Tasks *tasks, TM_String *buffers, const char *program, int *argc, char ***argv);
bool tm_db_register_tasks(const TM_Tasks *tasks, TM_String *string);
bool tm_db_execute_query(const char *query_buf);
char *tm_db_create_table();
void tm_debug_tasks(const TM_Tasks *tasks);

const char *tm_shift_args(int *argc, char ***argv);
void tm_usage(const char *program);

extern sqlite3 *db;

// strdup implementation
static inline char *tm_strdup(const char *src)
{
    size_t len = strlen(src);
    char *temp = (char *)malloc(len + 1);
    assert(temp != NULL && "source duplication Failed.");
    memcpy(temp, src, len + 1);
    temp[len] = '\0'; // NULL Terminate
    return temp;
}

#endif // TM_H_
