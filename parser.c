#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Helper to clean up string whitespace
static void trim(char *s) {
    if (!s) return;
    while (*s == ' ') s++;
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\r' || s[len-1] == '\n')) {
        s[len-1] = '\0';
        len--;
    }
}

static CommandType resolve_type(const char *cmd) {
    if (strcmp(cmd, "insert") == 0) return CMD_INSERT;
    if (strcmp(cmd, "delete") == 0) return CMD_DELETE;
    if (strcmp(cmd, "update") == 0) return CMD_UPDATE;
    if (strcmp(cmd, "search") == 0) return CMD_SEARCH;
    if (strcmp(cmd, "print") == 0) return CMD_PRINT;
    // "threads" is technically unknown for execution purposes
    return CMD_UNKNOWN;
}

int parse_commands(const char *path, Command **out_cmds, size_t *out_count) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;

    char line[256];
    size_t cap = 16;
    size_t count = 0;
    Command *cmds = malloc(cap * sizeof(Command));

    while (fgets(line, sizeof(line), f)) {
        // Handle line endings
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        // Make a copy for strtok
        char *line_copy = strdup(line);
        char *token = strtok(line_copy, ",");
        
        if (!token) {
            free(line_copy);
            continue;
        }

        trim(token);
        
        // 1. Check for "threads" command (Header) and skip it
        if (strcmp(token, "threads") == 0) {
            free(line_copy);
            continue;
        }

        CommandType type = resolve_type(token);
        if (type == CMD_UNKNOWN) {
            free(line_copy);
            continue;
        }

        // Resize array if needed
        if (count >= cap) {
            cap *= 2;
            cmds = realloc(cmds, cap * sizeof(Command));
        }

        Command *c = &cmds[count];
        c->type = type;
        // Defaults
        c->name[0] = '\0';
        c->salary = 0;
        c->priority = 0;

        // 2. Parse the next 3 tokens: Name, Salary, Priority
        // The input format seems to be strict: Command, Name, Salary, Priority
        char *arg_name = strtok(NULL, ",");
        char *arg_salary = strtok(NULL, ",");
        char *arg_priority = strtok(NULL, ",");

        if (arg_name) {
            trim(arg_name);
            strcpy(c->name, arg_name);
        }
        
        if (arg_salary) {
            trim(arg_salary);
            c->salary = atoi(arg_salary);
        }

        if (arg_priority) {
            trim(arg_priority);
            c->priority = atoi(arg_priority);
        }

        // Special case fixups based on command type if needed
        // For 'print', the name and salary are dummy 0s. 
        // The loop above handles this naturally: name becomes "0", salary becomes 0, priority becomes <val>.
        // This is fine because hashPrint ignores name/salary fields of the command struct.
        
        count++;
        free(line_copy);
    }

    fclose(f);
    *out_cmds = cmds;
    *out_count = count;
    return 0;
}