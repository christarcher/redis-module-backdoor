#include "cmd.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char *base64_encode(const unsigned char *data, size_t len) {
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_len;
    char *output;
    size_t i;
    size_t j;

    if (len == 0) {
        output = malloc(1);
        if (output != NULL) {
            output[0] = '\0';
        }
        return output;
    }

    output_len = 4 * ((len + 2) / 3);
    output = malloc(output_len + 1);
    if (output == NULL) {
        return NULL;
    }

    for (i = 0, j = 0; i < len; i += 3) {
        unsigned int octet_a = data[i];
        unsigned int octet_b = (i + 1 < len) ? data[i + 1] : 0;
        unsigned int octet_c = (i + 2 < len) ? data[i + 2] : 0;
        unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        output[j++] = table[(triple >> 18) & 0x3f];
        output[j++] = table[(triple >> 12) & 0x3f];
        output[j++] = (i + 1 < len) ? table[(triple >> 6) & 0x3f] : '=';
        output[j++] = (i + 2 < len) ? table[triple & 0x3f] : '=';
    }

    output[output_len] = '\0';
    return output;
}

char *run_cmd(const char *cmd) {
    static const char stderr_redirect[] = "(%s) 2>&1";
    unsigned char chunk[4096];
    size_t shell_cmd_len;
    char *shell_cmd;
    FILE *fp;
    unsigned char *output = NULL;
    size_t output_size = 0;
    size_t output_capacity = 0;
    char *encoded_output;

    shell_cmd_len = snprintf(NULL, 0, stderr_redirect, cmd);
    shell_cmd = malloc(shell_cmd_len + 1);
    if (shell_cmd == NULL) {
        return NULL;
    }

    snprintf(shell_cmd, shell_cmd_len + 1, stderr_redirect, cmd);

    fp = popen(shell_cmd, "r");
    free(shell_cmd);
    if (fp == NULL) {
        return NULL;
    }

    for (;;) {
        size_t bytes_read = fread(chunk, 1, sizeof(chunk), fp);

        if (bytes_read > 0) {
            if (output_size + bytes_read > output_capacity) {
                size_t new_capacity = output_capacity == 0 ? sizeof(chunk) : output_capacity;
                unsigned char *tmp;

                while (output_size + bytes_read > new_capacity) {
                    new_capacity *= 2;
                }

                tmp = realloc(output, new_capacity);
                if (tmp == NULL) {
                    free(output);
                    pclose(fp);
                    return NULL;
                }

                output = tmp;
                output_capacity = new_capacity;
            }

            memcpy(output + output_size, chunk, bytes_read);
            output_size += bytes_read;
        }

        if (bytes_read < sizeof(chunk)) {
            break;
        }
    }

    if (ferror(fp)) {
        free(output);
        pclose(fp);
        return NULL;
    }

    pclose(fp);

    encoded_output = base64_encode(output, output_size);
    free(output);

    return encoded_output;
}

int daemonize_cmd(const char *cmd) {
    int null_fd;
    int status;
    pid_t pid;
    pid_t grandchild_pid;

    pid = fork();
    if (pid < 0) {
        return -1;
    }

    if (pid == 0) {
        if (setsid() < 0) {
            _exit(1);
        }

        grandchild_pid = fork();
        if (grandchild_pid < 0) {
            _exit(1);
        }

        if (grandchild_pid > 0) {
            _exit(0);
        }

        null_fd = open("/dev/null", O_RDWR);
        if (null_fd < 0) {
            _exit(1);
        }

        if (dup2(null_fd, STDIN_FILENO) < 0 ||
            dup2(null_fd, STDOUT_FILENO) < 0 ||
            dup2(null_fd, STDERR_FILENO) < 0) {
            close(null_fd);
            _exit(1);
        }

        if (null_fd > STDERR_FILENO) {
            close(null_fd);
        }

        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        _exit(1);
    }

    if (waitpid(pid, &status, 0) < 0) {
        return -1;
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return -1;
    }

    return 0;
}
