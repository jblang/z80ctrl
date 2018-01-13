#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "altmon.h"
#include "bus.h"
#include "memory.h"
#include "z80.h"
#include "cli.h"

#define WHITESPACE " \t\r\n"
#define MAXBUF 80
#define MAXARGS 3

void cli_dump(char *argv[], int argc)
{
    uint16_t addr;
    uint16_t length;
    if (argc != 2) {
        printf("usage: dump <start> <length>\n");
        return;
    }
    sscanf(argv[0], "%x", &addr);
    sscanf(argv[1], "%x", &length);
    dump_mem(addr, length);
}

void cli_run(char *argv[], int argc)
{
    uint16_t addr;
    if (argc != 1) {
        printf("usage: run <address>\n");
        return;
    }
    sscanf(argv[0], "%x", &addr);
    z80_run(addr);
}

void cli_bus(char *argv[], int argc)
{
    puts(bus_status());
}

void cli_altmon(char *argv[], int argc)
{
    write_mem(0xf800, altmon_bin, altmon_bin_len);
    z80_run(0xf800);
}

void cli_help(char *argv[], int argc);

typedef struct _cli_entry {
    char *name;
    char *desc;
    void (*func)(char **, int);
} cli_entry;

cli_entry cli_cmds[] = {
    {"help", "list available commands", &cli_help},
    {"dump", "hex dump of memory range", &cli_dump},
    {"run", "execute code at address", &cli_run},
    {"bus", "display current bus status", &cli_bus},
    {"altmon", "run altmon 8080 monitor", &cli_altmon}
};

#define NUM_CMDS (sizeof(cli_cmds)/sizeof(cli_entry))

void cli_help(char *argv[], int argc)
{
    int i;
    printf("available commands:\n");
    for (i = 0; i < NUM_CMDS; i++) {
        printf("%s\t%s\n", cli_cmds[i].name, cli_cmds[i].desc);
    }
}

void cli_loop(void) {
    char buf[MAXBUF];
    char *cmd;
    char *argv[MAXARGS];
    int argc;
    int i;

    printf("type help to list available commands\n");
    for (;;) {
        printf("z80ctrl>");
        if (fgets(buf, sizeof buf - 1, stdin) != NULL) {
            if ((cmd = strtok(buf, WHITESPACE)) == NULL)
                continue;            
            for (argc = 0; argc < MAXARGS; argc++) {
                if ((argv[argc] = strtok(NULL, WHITESPACE)) == NULL)
                    break;
            }
            for (i = 0; i < NUM_CMDS; i++) {
                if (strcmp(cmd, cli_cmds[i].name) == 0) {
                    cli_cmds[i].func(argv, argc);
                    break;
                }
            }
            if (i == NUM_CMDS) {
                printf("unknown command: %s. type help for list.\n", cmd);
            }
        }
    }
}
