#ifndef TM_H_
#define TM_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include "./array.h"

#define TM_INITIAL_CAP  10
#define TM_BUFFER_LEN   1024
#define TM_PRIORITY_LEN 10

typedef enum _tm_priority {
    TM_LOW,
    TM_MEDIUM,
    TM_HIGH,
    TM_PRIORITY_COUNT,
} TM_Priority;

const char *tm_priority_as_cstr(TM_Priority p);
TM_Priority tm_priority_from_cstr(const char *p_cstr);

typedef struct _tm_task {
    char *message;
    unsigned int id;
    TM_Priority priority;
    bool done;
} TM_Task;

typedef ARRAY(TM_Task) TM_Tasks; // Incase the user provides more than 1 task through cli

// Database Functions
typedef enum _tm_query_type {
    TM_SELECT_ALL = 0,
    TM_SELECT_ONE,

    TM_DELETE_ALL,
    TM_DELETE_ONE,

    TM_INSERT,
    TM_UPDATE,

    TM_QUERY_COUNT,
} TM_QueryType;

typedef union _tm_query_data {
    // Querying Database
    char *select_all_rows;
    char *select_row;

    // Deleting Specific Row
    char *delete_row;
    char *delete_all_rows;

    // Inserting Data
    char *insert_row;

    // Updating A Row
    char *update_row;
} TM_QueryData;

typedef struct _tm_query {
    TM_QueryType type;
    TM_QueryData *statement;
} TM_Query;

void tm_sqlite3_version(void);

const char *tm_query_types_as_cstr(TM_QueryType t);
TM_QueryType tm_query_type_from_cstr(const char *type_as_cstr);

void tm_db_begin(const char *db_path, sqlite3 *db);
char *tm_db_query_task(TM_Query *query, const char *table, unsigned int *ID, const TM_Task *task, TM_Priority *priority);
void tm_db_end(sqlite3 *db);

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
