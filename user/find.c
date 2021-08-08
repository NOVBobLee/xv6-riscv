#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

#define WAITINGLINE_SIZE 1024
#define BUFFSIZ 30

char waitingLine[WAITINGLINE_SIZE];

void add_to_waitingLine(char *dir)
{
    static char *to_be_added = waitingLine;
    static int res_of_waitingLine = WAITINGLINE_SIZE;
    int length = strlen(dir);

    if (length + 1 > WAITINGLINE_SIZE) {
        fprintf(2, "No enough buff.\n");
        exit(2);
    }

    memcpy(to_be_added, dir, length);
    *(to_be_added + length) = 0;

    to_be_added += length + 1;
    res_of_waitingLine -= length + 1;
}

char *pop_from_waitingLine(void)
{
    static char *to_be_popped = waitingLine;
    int length = strlen(to_be_popped);
    char *pop = to_be_popped;

    to_be_popped += length + 1;
    return pop;
}

char *last_field_of(char *path)
{
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; --p)
        ;
    ++p;
    return p;
}

void find(char *origin, char *target)
{
    char path[BUFFSIZ + DIRSIZ], *p;
    int fd, waiting = 0, length;
    struct dirent de;
    struct stat st;

    add_to_waitingLine(origin);
    ++waiting;

    while (waiting > 0) {
        memcpy(path, pop_from_waitingLine(), BUFFSIZ);

        if (strcmp(last_field_of(path), target) == 0)
            printf("%s\n", path);

        if (stat(path, &st) < 0) {
            fprintf(2, "find: cannot stat %s\n", path);
            continue;
        }

        if (st.type == T_DIR) {
            if ((fd = open(path, 0)) < 0) {
                fprintf(2, "find: cannot open %s\n", path);
                continue;
            }

            if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(path)) {
                fprintf(2, "find: path too long\n");
                continue;
            }

            p = path + strlen(path);
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(struct dirent)) {
                if (de.inum < 0)
                    continue;
                if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                    continue;
                if ((length = strlen(de.name)) == 0)
                    break;

                memcpy(p, de.name, strlen(de.name) + 1);

                if (stat(path, &st) < 0) {
                    fprintf(2, "find: cannot stat %s\n", path);
                    continue;
                }

                if (st.type == T_DIR) {
                    add_to_waitingLine(path);
                    ++waiting;
                } else if (strcmp(de.name, target) == 0)
                    printf("%s\n", path);
            } // end while read
            close(fd);
            --waiting;
        } // end if type == dir
    } // end while waiting > 0

    return;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(2, "Usage: find <starting point> <target>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}

