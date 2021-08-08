#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
    int p[2];
    char buff[1];
    pipe(p);
    if (fork() == 0) {
        if (read(p[0], buff, sizeof(buff)) == 0) {
            fprintf(2, "%d: missed.\n", getpid());
            exit(1);
        }
        fprintf(1, "%d: received ping\n", getpid());
        write(p[1], "b", 1);
        exit(0);
    }
    else {
        write(p[1], "a", 1);
        wait(0);
        if (read(p[0], buff, sizeof(buff)) == 0) {
            fprintf(2, "%d: missed.\n", getpid());
            exit(1);
        }
        fprintf(1, "%d: received pong\n", getpid());
    }
    exit(0);
}
