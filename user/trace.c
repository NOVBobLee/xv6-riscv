#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv)
{
    int mask;
    
    if (argc < 3) {
        fprintf(2, "Usage: trace <syscall mask> <cmd>\n");
        exit(1);
    }

    mask = atoi(argv[1]);
    if (trace(mask) < 0) {
        fprintf(2, "trace failed\n");
        exit(1);
    }

    exec(argv[2], &argv[2]);
    
    exit(0);
}

