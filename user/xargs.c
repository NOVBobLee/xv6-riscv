#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"

void xargs(int argc, char *argv[])
{
    char buf[100], c, *p = buf, *new_argv[MAXARG];

    --argc;
    for (int i = 0; i < argc; ++i) {
        new_argv[i] = argv[i + 1];
    }

    new_argv[argc] = p;
    while (read(0, &c, sizeof(char)) == 1) {
        if (c == '\n') {
            *p++ = 0;
            new_argv[++argc] = p;
        }
        else
            *p++ = c;

        if (argc >= MAXARG)
            break;
    }
    *p = 0;

    if (fork() == 0)
        exec(new_argv[0], new_argv);

    return;
}

void xargsn(int argc, char *argv[])
{
    char buf[100], c, *buf_0 = buf, *p, *new_argv[MAXARG + 1];
    int readc = 1, argc_0 = argc - 3;

    int max_argc = atoi(argv[2]) < MAXARG ? \
        atoi(argv[2]) : MAXARG;
    max_argc += argc_0;

    for (int i = 0; i < argc_0; ++i) {
        new_argv[i] = argv[i + 3];
    }

    while (readc == 1) {
        argc = argc_0;
        p = buf_0;

        new_argv[argc] = p;
        while ((readc = read(0, &c, sizeof(char))) == 1) {
            if (c == '\n') {
                *p++ = 0;
                new_argv[++argc] = p;
            }
            else
                *p++ = c;

            if (argc >= max_argc)
                break;
        }
        *p = 0;

        /*
        printf("new_argv[]: ");
        for (int i = 0; i < argc; ++i) {
            printf("^%s$ ", new_argv[i]);
        }
        printf("\n");
        */

        if (argc > argc_0 && fork() == 0)
            exec(new_argv[0], new_argv);
        wait(0);
    } // end while (readc == 1)

    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command>\n");
        exit(1);
    }

    if (strcmp(argv[1], "-n") == 0) {
        if (argc < 4) {
            fprintf(2, "Usage: xargs -n <max_num_argc> <command>\n");
            exit(2);
        }

        xargsn(argc, argv);
    }
    else {
        xargs(argc, argv);
        wait(0);
    }

    exit(0);
}
