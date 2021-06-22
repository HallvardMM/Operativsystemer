#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <limits.h>

#define FILEPATH "dummyFile"

long totalBytesAlarm = 0;
long totalBytes = 0;

void compute_bandwidth(int signum)
{

    totalBytes += totalBytesAlarm;
    printf("Bandwith: %ld \n", totalBytesAlarm);
    totalBytesAlarm = 0;
    alarm(1);
}

void handle_USR1(int signum)
{
    printf("Total bytes sent over pipe: %ld \n", totalBytes);
}

void createDummyFile(int numberOfBytes)
{
    //Function to create a file of given size.
    int fd;
    int result;
    //Removes the dummy file if it exsists
    remove(FILEPATH);
    fd = open(FILEPATH, O_WRONLY | O_CREAT | O_EXCL, (mode_t)0777);
    if (fd == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    result = lseek(fd, numberOfBytes - 1, SEEK_SET);
    if (result == -1)
    {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }

    /* write just one byte at the end */
    result = write(fd, "", 1);
    if (result < 0)
    {
        close(fd);
        perror("Error writing a byte at the end of the file");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void fastPipe(int numberOfBytes)
{
    //Creates a pipe and forks where child writes to pipe
    //and parent reads from pipe
    int pipefd[2];
    pid_t cpid;
    char readbuffer[numberOfBytes];

    createDummyFile(numberOfBytes);

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (cpid == 0)
    {
        //Child writes to pipe
        int numberOfBytesWritten;
        while (1)
        {
            numberOfBytesWritten = write(pipefd[1], &FILEPATH, numberOfBytes);
            if (numberOfBytesWritten == -1)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }

            //This is not part of the task but is okay for debugging
            /*else
            {
                printf("Number of bytes written: %d\n", numberOfBytesWritten);
            }*/
        }
    }
    else
    {
        pid_t ppid = getpid();
        printf("Parent id %d \n", ppid);
        signal(SIGALRM, compute_bandwidth);
        signal(SIGUSR1, handle_USR1);
        alarm(1);
        //Parent read from pipe
        while (1)
        {
            int numberOfBytesRead;
            numberOfBytesRead = read(pipefd[0], readbuffer, numberOfBytes);
            if (numberOfBytesRead == -1)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
            else
            {
                totalBytesAlarm += numberOfBytesRead;
                printf("Number of bytes read: %d\n", numberOfBytesRead);
            }
        }
    }
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Argument should only be the number of bytes");
        exit(EXIT_FAILURE);
    }
    char *p;
    errno = 0;
    long conv = strtol(argv[1], &p, 10);

    if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN)
    {
        fprintf(stderr, "Error with argument %s", argv[1]);
        exit(EXIT_FAILURE);
    }
    else
    {
        int num;
        num = conv;
        fastPipe(num);
    }

    return 0;
}

/*
Answear to tasks:
 - The largest stable block size supported with unnamed pipes on our system is: 12230 bytes.
    If we used larger blocksize the number of bytes read jumped between numbers (and the most common number was 8192 bytes).
 - Highest bandwidth achieved with unamed pipes and block size: 4044191940 bytes/second = 4.0GB/s with block size 12230 bytes
 - The bandwith does change when using unamed pipes and starting several instances of this program at the same time.
    It changed from ca. 4044191940 bytes/second = 4.0GB/s to ca. 1035489640 bytes/second = 1.0GB/s when starting four instances.
*/
