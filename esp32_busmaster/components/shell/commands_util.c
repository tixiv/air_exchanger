/**
 * Utility functions
 *
 * Copyright (c) Thomas Kindler <mail@t-kindler.de>
 *
 * 2013-03-25: tk, GPIO test commands
 * 2013-03-25: tk, refactored parse_range from parameter.c
 * 2009-05-14: tk, added some shell commands
 * 2009-04-22: tk, added crc32 and statistics function
 * 2005-11-30: tk, improved hexdump with ascii output
 * 2002-01-10: tk, added strnbar function for progress bars
 * 2000-04-24: tk, initial implementation
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "command.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <malloc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <limits.h>

#include "ansi.h"

extern int stdin_available(void);

 /**
 * Generate a nice hexdump of a memory area.
 *
 * \param  mem     pointer to memory to dump
 * \param  length  how many bytes to dump
 */
void hexdump(const void *mem, unsigned length)
{
    puts("       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    0123456789ABCDEF");

    const unsigned char *src = (const unsigned char*)mem;
    for (unsigned i=0; i<length; i += 16, src += 16) {
        char  line[80], *t = line;

        t += sprintf(t, "%04x:  ", i);
        for (unsigned j=0; j<16; j++) {
            if (i+j < length)
                t += sprintf(t, "%02X", src[j]);
            else
                t += sprintf(t, "  ");
            *t++ = j & 1 ? ' ' : '-';
        }

        t += sprintf(t, "  ");
        for (unsigned j=0; j<16; j++) {
            if (i+j < length)
                *t++ = isprint(src[j]) ? src[j] : '.';
            else
                *t++ = ' ';
        }
        *t++ = 0;
        puts(line);
    }
}

// -------------------- Shell commands --------------------
//

/**
 * Execute a command periodically.
 *
 * \todo  make the delay interruptible
 * \todo  use system() instead of exec()
 */
int cmd_watch(int argc, char *argv[])
{
    int interval = 100;
    if (argc < 2)
        goto usage;

    if (!strcmp(argv[1], "-n")) {
        if (argc < 4)
            goto usage;
        interval = atoi(argv[2]);
        if (interval < 0)
            goto usage;

        argc -= 2;
        argv += 2;
    }

    for (;;) {
        printf(ANSI_CLEAR ANSI_HOME "every %dms:", interval);
        for (int i=1; i<argc; i++)
            printf(" %s", argv[i]);
        printf("\n\n");

        cmd_exec(argc - 1, &argv[1]);
        fflush(stdout);

        int delay = interval;
        do {
            while (stdin_available()) {
                if (getchar() == 3) // CTRL-C
                    return 0;
            }

            vTaskDelay(delay > 100 ? 100 : delay);
            delay -= 100;
        } while (delay > 0);
    }

    return 0;

usage:
    printf("usage: %s [-n <ms>] <command>\n", argv[0]);
    return -1;
}


static int cmd_dump(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
        goto usage;

    char     *endp;
    void     *addr = (void*)strtoul(argv[1], &endp, 0);
    unsigned  len  = 256;

    if (*endp)
        goto usage;

    if (argc == 3) {
        len = strtoul(argv[2], &endp, 0);
        if (*endp)
            goto usage;
    }

    hexdump(addr, len);
    return 0;

usage:
    printf("Usage: %s <addr> [len]\n", argv[0]);
    return -1;
}

/**
 * Show FreeRTOS runtime statistics.
 */
static int cmd_ps(int argc, char *argv[])
{
    int n = uxTaskGetNumberOfTasks();
    TaskStatus_t *stats = malloc(n * sizeof(TaskStatus_t));

    if (!stats) {
        printf("can't alloc buffer\n");
        return -1;
    }

    uint32_t  total_run_time;
    n = uxTaskGetSystemState(stats, n, &total_run_time);

    printf(" # Pri State Name             CPU Time   %%   Stack\n");

    for (int i=0; i<n; i++) {
        char status;
        switch(stats[i].eCurrentState) {
        case eReady:     status = 'R'; break;
        case eBlocked:   status = 'B'; break;
        case eSuspended: status = 'S'; break;
        case eDeleted:   status = 'D'; break;
        default:         status = '?'; break;
        }

        printf("%2u %3u %-5c %-16s %10lu %3lu %5lu\n",
            stats[i].xTaskNumber, stats[i].uxCurrentPriority,
            status, stats[i].pcTaskName, stats[i].ulRunTimeCounter,
            (uint32_t)(stats[i].ulRunTimeCounter * 100ULL / total_run_time),
            stats[i].usStackHighWaterMark
        );
    }

    free(stats);
    return 0;
}

static int cmd_sysinfo(int argc, char *argv[])
{
    extern char _etext;    // end address of the .text section

    // extern char *__brkval;

    printf("_etext   = %p\n", &_etext  );
    // printf("heap avail       = %10d\n", (char*)__get_MSP() - __brkval);
    malloc_stats();

    return 0;
}

SHELL_CMD(watch,     (cmdfunc_t)cmd_watch,      "execute periodically")
SHELL_CMD(dump,      (cmdfunc_t)cmd_dump,       "dump memory area")
SHELL_CMD(sysinfo,   (cmdfunc_t)cmd_sysinfo,    "show system information")
SHELL_CMD(ps,        (cmdfunc_t)cmd_ps,         "show tasks")
