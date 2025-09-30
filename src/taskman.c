#include <sqlite3.h>
#include "task.h"

const char *shift_args(int *argc, char ***argv)
{
    const char *result = **argv;
    (*argc)--;
    (*argv)++;
    return result;
}

void usage(const char *program)
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

bool parse_integer(const char *cstr_i, int *number)
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

bool parse_cmd(const char *program, int *argc, char ***argv)
{
    while (*argc > 0) {
        const char *current_flag = shift_args(argc, argv);
        if (strcmp(current_flag, "--version") == 0 || strcmp(current_flag, "-v") == 0) {
            our_sqlite3_version();
        } else if (strcmp(current_flag, "--task") == 0) {
            if (*argc > 0) {
                const char *message = shift_args(argc, argv);
                if (strcmp(message, "--priority") == 0 || strcmp(message, "--p") == 0) {
                    usage(program);
                    fprintf(stderr, "\nERROR: NO Task Provided after `%s` flag.\n",current_flag);
                    return false;
                } else {
                    printf("TASK: %s\n", message);
                }
                if (*argc > 0) {
                    const char *priority = shift_args(argc, argv);
                    if (strcmp(priority, "--priority") == 0 || strcmp(priority, "--p") == 0) {
                        if (*argc > 0) {
                            const char *priority_level = shift_args(argc, argv);
                            int number;
                            if (parse_integer(priority_level, &number)) {
                                if (number < 3 && number >= 0) {
                                    printf("PRIORITY: %s\n", priority_as_cstr(number));
                                } else {
                                    usage(program);
                                    fprintf(stderr, "\nERROR: UnKnown Priority Level `%d` Provided.\n", number);
                                    return false;
                                }
                            } else {
                                printf("PRIORITY: %s\n", priority_level);
                            }
                        } else {
                            usage(program);
                            fprintf(stderr, "\nERROR: No Priority Level Provided.\n");
                            return false;
                        }
                    }
                } else {
                    // TODO: Consider Giving the id instead of message in Future
                    fprintf(stderr, "\nWARN: No Priority Provided for task `%s`. Defaulting to LOW priority\n", message);

                    // TODO: Add Additional Args in Future
                    return true; // Finished parsing , got task and priority return early for now,
                }
            } else {
                usage(program);
                fprintf(stderr, "\nERROR: No Task Provided.\n");
                return false;
            }
        } else {
            usage(program);
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
    const char *program = shift_args(&argc, &argv);
    if (argc == 0) {
        usage(program);
        return 1;
    }
    if (!parse_cmd(program, &argc, &argv)) return 1;
    return 0;
}


// TODO: `tm` namespacing
