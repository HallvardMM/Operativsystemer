# Practical4


## Run
gcc wish.c -o wish

./wish

## Debug
gcc wish.c -g

gdb ./a.out

run

## Printing strings from pointer-array
`for (int w = 0; w < sizes[0]; w++)
{ //Test
    char *p = commands[w];
    while (*p != '\0')
    {
        printf("%c", *p++);
    }
    printf(" ");
}`