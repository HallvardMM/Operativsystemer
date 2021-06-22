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

#define FILEPATH "dummyFileNamed"

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
        close(fd);
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
    pid_t cpid;
    char readbuffer[numberOfBytes];

    char *myfifo = "myfifo";

    createDummyFile(numberOfBytes);

    if (remove(myfifo) == -1)
    {
        printf("Remove myfifo failed");
    }

    if (mkfifo(myfifo, (mode_t)0777) == -1)
    {
        perror("mkfifo");
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
        int fdWrite;
        int numberOfBytesWritten;
        fdWrite = open(myfifo, O_CREAT | O_WRONLY);

        if (fdWrite == -1)
        {
            perror("open write");
            close(fdWrite);
            exit(EXIT_FAILURE);
        }
        while (1)
        {
            numberOfBytesWritten = write(fdWrite, &FILEPATH, numberOfBytes);
            if (numberOfBytesWritten == -1)
            {
                perror("write");
                close(fdWrite);
                exit(EXIT_FAILURE);
            }
            //This is not part of the task but is okay for debugging
            /*else
            {
                printf("Number of bytes written: %d\n", numberOfBytesWritten);
            }*/
        }
        close(fdWrite);
    }
    else
    {

        int fdRead;
        int numberOfBytesRead;
        pid_t ppid = getpid();

        printf("Parent id %d \n", ppid);
        signal(SIGALRM, compute_bandwidth);
        signal(SIGUSR1, handle_USR1);
        alarm(1);
        //Parent read from pipe
        fdRead = open(myfifo, O_CREAT | O_RDONLY);
        if (fdRead == -1)
        {
            perror("open read");
            close(fdRead);
            exit(EXIT_FAILURE);
        }
        while (1)
        {
            numberOfBytesRead = read(fdRead, readbuffer, numberOfBytes);
            if (numberOfBytesRead == -1)
            {
                perror("read");
                close(fdRead);
                exit(EXIT_FAILURE);
            }
            else
            {
                totalBytesAlarm += numberOfBytesRead;
                printf("Number of bytes read: %d\n", numberOfBytesRead);
            }
        }
        close(fdRead);
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
 - The largest stable block size supported with named pipes on our system is: 12230 bytes.
    If we used larger blocksize the number of bytes read jumped between numbers (and the most common number then was 8192 bytes).
    This was the same behvoir we got with unnamed pipes, but the bandwith differs.
 - Highest bandwidth achieved with named pipes and block size: 2896125150 bytes/second = 2.9GB/s with block size 12230 bytes
 - The bandwith with named pipes changes when starting several instances of this program at the same time.
    It changed from 2896125150 bytes//second = 2.9GB/s to 860545024 bytes//second = 0.86GB/s when starting four instances.
*/
