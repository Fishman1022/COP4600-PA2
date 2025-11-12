#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stddef.h>

typedef enum { CMD_INSERT, CMD_DELETE, CMD_SEARCH, CMD_PRINT, CMD_UNKNOWN } CommandType;

typedef struct {
    CommandType type;
    char name[64];
    uint32_t salary;
    int priority;
} Command;

int parse_commands(const char *path, Command **out_cmds, size_t *out_count);

#endif
