#include "kernel/types.h"
#include "user/user.h"

int main(void)
{
    printf("up %d ticks\n", uptime());
    exit(0);
}
