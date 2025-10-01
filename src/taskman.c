#include <sqlite3.h>

#include "tm.h"

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
    fprintf(out, "  --task <message>    Specify task description (required)\n");
    fprintf(out, "  --priority <value>  Set task priority (required)\n");
    fprintf(out, "                      Values: 0|LOW, 1|MEDIUM, 2|HIGH\n");
    fprintf(out, "  --list, -l         List the Available Tasks\n");
    fprintf(out, "  --version, -v      Print out the database details\n");
    fprintf(out, "  --help, -h         Show this help message\n\n");
    fprintf(out, "EXAMPLES:\n");
    fprintf(out, "  %s --task \"Finish project\" --priority HIGH\n", program);
    fprintf(out, "  %s --task \"Buy groceries\" --priority 1\n\n", program);
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

bool tm_parse_cli(TM_Tasks *tasks, const char *program, int *argc, char ***argv)
{
    while (*argc > 0) {
        const char *current_flag = tm_shift_args(argc, argv);
        if (strcmp(current_flag, "--version") == 0 || strcmp(current_flag, "-v") == 0) {
            tm_sqlite3_version();
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
                    // TODO: Consider Giving the id instead of message in Future
                    fprintf(stderr, "\nWARN: No Priority Provided for task `%s`. Defaulting to LOW priority\n", message);

                    // TODO: Add Additional Args in Future
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

int main(int argc, char **argv)
{
    const char *program = tm_shift_args(&argc, &argv);
    if (argc == 0) {
        tm_usage(program);
        return 1;
    }

    TM_Tasks tasks = {0};
    if (!tm_parse_cli(&tasks, program, &argc, &argv)) return 1;

    for (uint32_t i = 0; i < tasks.count; ++i) {
        TM_Task *task = &tasks.items[i];
        fprintf(stdout, "[INFO] ID: %d, Task: `%s` Priority: `%s` Done: %s.\n", task->id, task->message, tm_priority_as_cstr(task->priority), task->done == 0 ? "false" : "true");
    }

    array_delete(&tasks);
    return 0;
}
