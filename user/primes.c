#include "kernel/types.h"
#include "user/user.h"

#define READ 0
#define WRITE 1

const uint INT_LEN = sizeof(int);

/**
 * Read the first data from left pipe and print it (it's a prime)
 * @param leftpipe pipe array [read, write]
 * @param dst store the read data
 * @return read success, return 0; else return -1
 */

int leftpipe_first_data(int leftpipe[2], int *dst)
{
    // read INT_LEN to dst
    if (read(leftpipe[READ], dst, INT_LEN) == INT_LEN)
    {
        // print the prime
        printf("prime %d\n", *dst);
        return 0;
    }
    // read failed
    return -1;
}

/**
 * Read data from left pipe and transmit those who can by divided by first
 * @param leftpipe input pipe
 * @param rightpipe output pipe
 * @param first current prime
 */
void transmit_data(int leftpipe[2], int rightpipe[2], int first)
{
    int data;
    // read from leftpipe
    while (read(leftpipe[READ], &data, INT_LEN) == INT_LEN)
    {
        // write to the rightpipe (transmit)
        if (data % first)
        {
            write(rightpipe[WRITE], &data, INT_LEN);
        }
    }
    close(leftpipe[READ]);
    close(rightpipe[WRITE]);
}
/**
 * Recursively read the data in the pipe and output primes
 * @param leftpipe input pipe
 */
__attribute__((noreturn)) void primes(int leftpipe[2])
{
    // read only, no write
    close(leftpipe[WRITE]);
    int first;
    // read first data from leftpipe
    if (leftpipe_first_data(leftpipe, &first) == 0)
    {
        int p[2];
        pipe(p);
        // filter out numbers and transmit
        transmit_data(leftpipe, p, first);
        int pid = fork();
        if (pid == 0)
        {
            // child process recursively call primes to handle new pipes
            primes(p);
        }
        else
        {
            // wait for child process to finish
            close(p[READ]);
            wait(0);
        }
    }
    exit(0);
}
int main(int argc, char const *argv[])
{
    int p[2];
    pipe(p);
    // up to 35
    for (int i = 2; i <= 35; i++)
    {
        // write to the pipe
        write(p[WRITE], &i, INT_LEN);
    }
    int pid = fork();
    if (pid == 0)
    {
        primes(p);
    }
    else
    {
        close(p[WRITE]);
        close(p[READ]);
        wait(0);
    }
    exit(0);
}