#include <stdio.h>

int rec_sum = 0;
int counter = 0;
int sum_n(int add)
{
    if (add > 0)
    {
        rec_sum += add;

        add--;
        counter++;

        sum_n(add);
    }
    else
    {
        return rec_sum;
    }
}
int main()
{
    int initial_value = 65143;
    int sum = sum_n(initial_value);
    printf("\n the sum of numbers from 1 to %d is: %d.", initial_value, sum);
}