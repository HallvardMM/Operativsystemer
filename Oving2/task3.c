#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>

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
    pid_t pid, terminated;
    for (;;)
    {
        printf("Give me the delay time: \n");
        scanf("%d", &sec);
        terminated = waitpid(-1, NULL, WNOHANG);
        while (terminated > 0)
        // Since the termination of child is placed here, as specified in the task, there will be one zombie process until a new process is started
        {
            printf("terminated child %d \n", terminated);
            terminated = waitpid(-1, NULL, WNOHANG);
        }
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