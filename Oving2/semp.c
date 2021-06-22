#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem;
int val;

int main(void)
{
    sem_init(&sem, 1, 4);
    for (;;)
    {
        printf("X\n");
        wait(&sem);
        sem_getvalue(&sem, &val);
        printf("semvalue %d", val);
    }
    printf("X\n");
    return 0;
}