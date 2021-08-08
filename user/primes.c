#include "kernel/types.h"
#include "user/user.h"

/* redirect (fd) to (to_fd) */
void fd_change_from(int fd, int to_fd)
{
    close(fd);
    dup(to_fd);
    close(to_fd);
}

/*
 * parent: (i) change stdout from (console) to (pipe[1]) 
 *         (ii) generate 2, 3, 4, ...35 into pipe 
 *         (iii) wait for all childs exit
 * child: change stdin from (console) to (pipe[0])
 */ 
void num_generator(void)
{
    int num;
    int _pipe[2];

    pipe(_pipe);

    if (fork() > 0) {
        close(_pipe[0]);
        fd_change_from(1, _pipe[1]);

        for (num = 2; num < 36; ++num)
            write(1, &num, sizeof(int));
        close(1);

        while(wait(0) > 0);
        exit(0);
    } // parent
    
    close(_pipe[1]);
    fd_change_from(0, _pipe[0]);
    return;
}

void sieve_by(int prime);

int main(void)
{
    int prime;

    num_generator();

    /*
     * (i) catch the first element in the pipe as prime
     * (ii) sieve the remaining
     */
    while (read(0, &prime, sizeof(int)) != 0) {
        printf("prime %d\n", prime);
        sieve_by(prime);
    }
    close(0);

    exit(0);
}

/*
 * parent: (i) change stdout from (console) to (pipe[1])
 *         (ii) sieve pipe by prime
 *         (iii) pass sieved numbers into pipe
 * child: change stdin from (old_pipe[0]) to (new_pipe[0])
 */
void sieve_by(int prime)
{
    int num;
    int _pipe[2];

    pipe(_pipe);

    if (fork() > 0) {
        close(_pipe[0]);
        fd_change_from(1, _pipe[1]);
        
        while (read(0, &num, sizeof(int)) != 0) {
            if (num % prime != 0)
                write(1, &num, sizeof(int));
        }
        close(1);
        close(0);

        exit(0);
    } // parent
    
    close(_pipe[1]);
    fd_change_from(0, _pipe[0]);
    return;
}
