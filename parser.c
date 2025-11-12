#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim(char *s) {
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n-1])) s[--n] = '\0';
    size_t i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s+i, n - i + 1);
}

static CommandType parse_type(const char *tok) {
    if (!tok) return CMD_UNKNOWN;
    if (strcasecmp(tok, "insert") == 0) return CMD_INSERT;
    if (strcasecmp(tok, "delete") == 0) return CMD_DELETE;
    if (strcasecmp(tok, "search") == 0) return CMD_SEARCH;
    if (strcasecmp(tok, "print")  == 0) return CMD_PRINT;
    return CMD_UNKNOWN;
}

int parse_commands(const char *path, Command **out_cmds, size_t *out_count) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    size_t cap = 32, n = 0;
    Command *arr = malloc(sizeof(Command)*cap);
    if (!arr) { fclose(f); return -1; }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (!*line) continue;
        char *a = strtok(line, ",");
        char *b = strtok(NULL, ",");
        char *c = strtok(NULL, ",");
        char *d = strtok(NULL, ",");
        Command cmd = {0};
        cmd.type = parse_type(a);
        if (cmd.type == CMD_UNKNOWN) continue;
        if (cmd.type == CMD_INSERT) {
            if (!b || !c || !d) continue;
            strncpy(cmd.name, b, sizeof(cmd.name)-1);
            cmd.salary   = strtoul(c, NULL, 10);
            cmd.priority = atoi(d);
        } else if (cmd.type == CMD_DELETE || cmd.type == CMD_SEARCH) {
            if (!b || !c) continue;
            strncpy(cmd.name, b, sizeof(cmd.name)-1);
            cmd.priority = atoi(c);
        } else if (cmd.type == CMD_PRINT) {
            if (!b) continue;
            cmd.priority = atoi(b);
        }
        if (n == cap) {
            cap *= 2;
            arr = realloc(arr, sizeof(Command)*cap);
            if (!arr) { fclose(f); return -1; }
        }
        arr[n++] = cmd;
    }
    fclose(f);
    *out_cmds = arr;
    *out_count = n;
    return 0;
}
