#include "./tm.h"

int main(int argc, char **argv)
{
    int result = 0;
    static const char *test_db = "test.db";
    tm_db_begin(test_db);

    const char *program = tm_shift_args(&argc, &argv);
    if (argc == 0) {
        tm_usage(program);
        result = 1;
        goto defer;
    }

    TM_Tasks tasks = {0};
    TM_String bufs = {0};
    if (!tm_parse_cli(&tasks, &bufs, program, &argc, &argv)) {
        result = 1;
        goto defer;
    }

    char *create = tm_db_create_table();
    if (!tm_db_execute_query(create)) {
        free(create);
        result = 1;
        goto defer;
    }
    free(create);

    TM_String str = {0};
    if (tasks.count > 0) {
        if (!tm_db_register_tasks(&tasks, &str)) {
            result = 1;
            goto defer;
        }
        for (uint32_t i = 0; i < str.count; ++i) {
            if(!tm_db_execute_query(str.items[i])) {
                free(str.items[i]);
                result = 1;
                goto defer;
            } else {
                free(str.items[i]);
            }
        }
    }

    if (bufs.count > 0) {
        for (size_t i = 0; i < bufs.count; ++i) {
            char *buf = bufs.items[i];
            if (buf && tm_db_execute_query(buf)) {
                free(buf);
            } else {
                free(buf);
                result = 1;
                goto defer;
            }
        }
    }

    result = 0; goto defer; // on success
defer:
    if (db) tm_db_end(db); // close sqlite3 coonection
    if (bufs.count > 0 && bufs.items) array_delete(&bufs); // free
    if (tasks.count > 0 && tasks.items) array_delete(&tasks); // free
    if (str.count > 0 && str.items) array_delete(&str); // free
    return result;
}
