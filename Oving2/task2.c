#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

void manyclocks(int sec)
{
    sleep(sec);
    signed int id = getpid();
    printf("RING RING %d\n", id);
    exit(0);
}

int main()
{
    int sec;
    pid_t pid;
    for (;;)
    {
        printf("Give me the delay time: \n");
        scanf("%d", &sec);
        pid = fork();
        if (pid == 0)
        {
            manyclocks(sec);
        }
        else
        {
            printf("childID %d \n", pid);
        }
    };
    return 0;
}