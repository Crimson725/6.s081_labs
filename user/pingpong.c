#include "kernel/types.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

int main(int argc, char const *argv[])
{
    char buf = 'P';
    int fd_child2parent[2];
    int fd_parent2child[2];
    pipe(fd_child2parent);
    pipe(fd_parent2child);

    int pid = fork();
    int exit_status = 0;
    if (pid < 0)
    {
        fprintf(2, "fork() error!\n");
        close(fd_child2parent[READ]);
        close(fd_child2parent[WRITE]);
        close(fd_parent2child[READ]);
        close(fd_parent2child[WRITE]);
        exit(1);
    }
    else if (pid == 0)
    {
        // in child process
        // child write to parent

        // parent2child write close
        // parent only reads
        close(fd_parent2child[WRITE]);

        // child2parent read close
        //  child only writes
        close(fd_child2parent[READ]);

        if (read(fd_parent2child[READ], &buf, sizeof(char)) != sizeof(char))
        {
            fprintf(2, "child read() error!\n");
            exit_status = 1;
        }
        else
        {
            fprintf(1, "%d: received ping\n", getpid());
        }
        // reset buf
        buf = 'P';
        if (write(fd_child2parent[WRITE], &buf, sizeof(char)) != sizeof(char))
        {
            fprintf(2, "child write() error!\n");
            exit_status = 1;
        }
        close(fd_parent2child[READ]);
        close(fd_child2parent[WRITE]);
        exit(exit_status);
    }
    else
    {
        close(fd_parent2child[READ]);
        close(fd_child2parent[WRITE]);
        if (write(fd_parent2child[WRITE], &buf, sizeof(char)) != sizeof(char))
        {
            fprintf(2, "parent write() error!\n");
            exit_status = 1;
        }
        if (read(fd_child2parent[READ], &buf, sizeof(char)) != sizeof(char))
        {
            fprintf(2, "parent read() error!\n");
            exit_status = 1;
        }
        else
        {
            fprintf(1, "%d: received pong\n", getpid());
        }
        close(fd_parent2child[WRITE]);
        close(fd_child2parent[READ]);
        exit(exit_status);
    }
}