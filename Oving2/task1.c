#include <stdio.h>
#include <string.h>
#include <unistd.h>

void alarmclock(int sec)
{
    sleep(sec);
    printf("RING RING \n");
}

int main()
{
    int sec;
    for (;;)
    {
        printf("Give me the delay time: \n");
        scanf("%d", &sec);
        alarmclock(sec);
    };
    return 0;
}