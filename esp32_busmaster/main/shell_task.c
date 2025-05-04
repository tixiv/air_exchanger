#include "shell_task.h"

#include "readline.h"
#include "command.h"

#include <sys/ioctl.h>
#include <lwip/sockets.h>
#include <string.h>

// Custom write callback for fwopen()
static int telnet_write(void *cookie, const char *buf, int size)
{
    int fd = *(int*)cookie;

    return send(fd, buf, size, 0);
}

static int telnet_read(void *cookie, char *buf, int size)
{
    int fd = *(int*)cookie;

    return recv(fd, buf, size, 0);
}

static __thread int current_stdin_fd = -1;

int stdin_available(void) {
    if (current_stdin_fd < 0) return 0;

    int count = 0;
    if (ioctl(current_stdin_fd, FIONREAD, &count) < 0) {
        return 0; // or log an error
    }
    return count;
}

void shell_task(void *pvParameters)
{
    int fd = *(int*)pvParameters;
    current_stdin_fd = fd;

    FILE *telnet_stream = funopen(pvParameters, telnet_read, telnet_write, NULL, NULL);

    stdout = telnet_stream;
    stderr = telnet_stream;
    stdin = telnet_stream;

    struct rl_history  history = { 0 };
    history.size   = 256;
    history.buf    = malloc(history.size);
    history.buf[0] = 0;

    // save some useful commands..
    //
    hist_add(&history, "help");

    char prompt[16], line[80];
    sprintf(prompt, "> ");

    printf("Welcome to the shell\n");

    for(;;) {
        line[0] = '\0';
        int n = readline(prompt, line, sizeof line, &history);
        if (n < 0)
            break;

        cmd_system(line);
    }

    free(history.buf);
    fclose(telnet_stream);
    close(fd);
    free(pvParameters);

    vTaskDelete(NULL);
}