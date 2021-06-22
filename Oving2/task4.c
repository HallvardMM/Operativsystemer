#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>

void manyclocks(int sec)
{
    int timeleft = sleep(sec);
    while (timeleft != 0) //Check if sleep is done or exited before it was done
    {
        timeleft = sleep(timeleft);
    }
    signed int id = getpid(); //These functions are always successful
    int checkRingPrint = printf("RING RING %d\n", id);
    if (checkRingPrint < 0) //maybe overkill to errorcheck print
    {
        perror("The ring ring print failed: ");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int main()
{
    int sec;
    pid_t pid, terminated;
    for (;;)
    {
        int checkDelayPrint = printf("Give me the delay time (s): \n");
        if (checkDelayPrint < 0) //maybe overkill to errorcheck print
        {
            perror("The delaytime print failed: ");
            exit(EXIT_FAILURE);
        }

        int scanfReturn = scanf("%d", &sec);
        if (!scanfReturn || sec < 0)
        {
            printf("Invalid argument \n"); //errorno is not set in this condition
            exit(EXIT_FAILURE);
        }
        if ((getchar()) != '\n')
        {
            printf("Too many arguments \n"); //errorno is not set in this condition
            exit(EXIT_FAILURE);
        }

        terminated = waitpid(-1, NULL, WNOHANG);
        if (terminated == -1)
        {
            if (errno != ECHILD)
            {
                perror("Error with waitpid: ");
                exit(EXIT_FAILURE);
            }
        }
        while (terminated > 0)
        {
            int checkChildPrint = printf("terminated child %d \n", terminated);
            if (checkChildPrint < 0) //maybe overkill to errorcheck print
            {
                perror("The terminated child print failed: ");
                exit(EXIT_FAILURE);
            }
            terminated = waitpid(-1, NULL, WNOHANG);
            if (terminated == -1)
            {
                if (errno != ECHILD)
                //Echild The calling process does not have any unwaited-for children.
                {
                    perror("Error with waitpid: ");
                    exit(EXIT_FAILURE);
                }
            }
        }
        pid = fork();
        if (pid == 0)
        {
            manyclocks(sec);
        }
        else if (pid == -1) //The fork failes
        {
            perror("The fork failed: ");
            exit(EXIT_FAILURE);
        }
        else
        {
            int checkChildIDPrint = printf("childID %d \n", pid);
            if (checkChildIDPrint < 0) //maybe overkill to errorcheck print
            {
                perror("The print of childID failed: ");
                exit(EXIT_FAILURE);
            }
        }
    };
    return 0;
}