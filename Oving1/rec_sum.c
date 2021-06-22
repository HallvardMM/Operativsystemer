#include <stdio.h>
#include <string.h>

int uninitialized_global_int;
int initialized_global_int = 10;
float uninitialized_global_float;
float initialized_global_float = 10.0;
char uninitialized_global_char;
char initialized_global_char = 'a';
int uninitialized_global_array[5];
int initialized_global_array[5] = {1, 2, 3, 4, 5};

int sum_n(int n1)
{
   int res;
   if (n1 == 1)
   {
      return (1);
   }
   else
   {
      res = n1 + sum_n(n1 - 1);
   }
   return (res);
}

int main()
{
   int uninitialized_local_int;
   int initialized_local_int = 10;
   float uninitialized_local_float;
   float initialized_local_float = 10.0;
   char uninitialized_local_char;
   char initialized_local_char = 'a';
   int uninitialized_local_array[5];
   int initialized_local_array[5] = {1, 2, 3, 4, 5};

   int n1 = 30;
   int sum = sum_n(n1);
   printf("The sum of numbers from 1 to %d is %d.\n", n1, sum);
   printf("Address of uninitialized_global_int is %p\n", &uninitialized_global_int);
   printf("Address of initialized_global_int is %p\n", &initialized_global_int);
   printf("Address of uninitialized_global_float is %p\n", &uninitialized_global_float);
   printf("Address of initialized_global_float is %p\n", &initialized_global_float);
   printf("Address of uninitialized_global_char is %p\n", &uninitialized_global_char);
   printf("Address of initialized_global_char is %p\n", &initialized_global_char);
   printf("Address of uninitialized_global_array is %p\n", &uninitialized_global_array);
   printf("Address of initialized_global_array is %p\n", &initialized_global_array);
   printf("\n");
   printf("Address of uninitialized_local_int is %p\n", &uninitialized_local_int);
   printf("Address of initialized_local_int is %p\n", &initialized_local_int);
   printf("Address of uninitialized_local_float is %p\n", &uninitialized_local_float);
   printf("Address of initialized_local_float is %p\n", &initialized_local_float);
   printf("Address of uninitialized_local_char is %p\n", &uninitialized_local_char);
   printf("Address of initialized_local_char is %p\n", &initialized_local_char);
   printf("Address of uninitialized_local_array is %p\n", &uninitialized_local_array);
   printf("Address of initialized_local_array is %p\n", &initialized_local_array);
}

/*
Task a)
It will depend on how the function is implemented. In rec_sum we didn't use global variables and we got: 
   - 43430 which gives the sum 943104165 before we added printf.
In we also made another recursiv function that used global variables and got:
   - 65143 which gives the sum 2121837796
If we have a larger number the computer failes to run since it meets recursion depth and gets a stackoverflow. 
It exits with code=3221225725 which is the errorcode for stackoverflow on Windows.
This stackoverflow leads to a segmentation fault.

Task b)
The distance in bytes depends on the type. 
We see that int and float uses 4 bytes.
We see that char uses 1 byte.
The uninitialized_local_array uses the size of the object head which is 8 bytes.
The array uses 5*4=20 bytes since it is 5 int with size 4 bytes.
The address of an variable will also be moved so that it is aligned with an address that is devidable with it's size.
Memory aligment of an int with size four leads to an address that ends with 0,4,8,12.

Task c)
A global int variable is located at a completely different address than the local ones since
the local variables are stored on the stack and global variables are stored in the Block Storage Segment (BSS) or data segment.
If the global variable is uninitialized it is stored in BSS and if it is initialized it is stored in data segment.

Task d)
The address of a local variable in the recursive function decreases since it is stored 
on the stack and the stack starts on higher addresses and grows down to lower addresses.
This makes it so that local variables will get lower addresses the higher the recursion is.

*/