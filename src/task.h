#ifndef TASK_H_
#define TASK_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#define INITIAL_CAP  10
#define BUFFER_LEN   1024
#define PRIORITY_LEN 10

typedef enum {
    LOW,
    MEDIUM,
    HIGH,
    PRIORITY_COUNT,
} Priority;

const char *priority_as_cstr(Priority p);

typedef struct _task {
    char *message;
    unsigned int id;
    Priority priority;
    bool done;
} Task;

// Database Functions
typedef enum {
    DB_SELECT_ALL = 0,
    DB_SELECT_ONE,

    DB_DELETE_ALL,
    DB_DELETE_ONE,

    DB_INSERT,
    DB_UPDATE,

    QUERY_TYPE_COUNT,
} Query_Type;

typedef union {
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
} Query_Data;

typedef struct {
    Query_Type type;
    Query_Data *statement;
} Query;

void our_sqlite3_version();
Query_Type map_query_to_cstr(const char *type_as_cstr);
char *query_task(Query *query, const char *table, unsigned int *ID, const Task *task, Priority *priority);

// strdup implementation
static inline char *strdup_custom(const char *source)
{
    size_t len = strlen(source);
    char *temp = (char *)malloc(len + 1);
    assert(temp != NULL && "source duplication Failed.");
    memcpy(temp, source,len + 1);
    return temp;
}

#endif // TASK_H_
