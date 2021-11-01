#include "kernel/types.h"
#include "user/user.h"

// lab pgtbl
// usyscall page
// struct { uint pid; };
#define USYSCALL (0x3fffffdL << 12)

int main(void)
{
    printf("pid: %d\nupid: %d\n", getpid(), *(int *const)USYSCALL);
    exit(0);
}
