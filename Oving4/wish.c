#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUMBER_OF_STRING 12
#define MAX_STRING_SIZE 64
// Change to the part you want to run
#define PART1 0
#define PART2 0
#define PART3 0
#define PART4 0
#define PART5 1

char Junk[16]; // buffer for discarding excessive user input
#define FLUSH_STDIN(x)                              \
    {                                               \
        if (x[strlen(x) - 1] != '\n')               \
        {                                           \
            do                                      \
                fgets(Junk, 16, stdin);             \
            while (Junk[strlen(Junk) - 1] != '\n'); \
        }                                           \
        else                                        \
            x[strlen(x) - 1] = '\0';                \
    }

void split_string(char *commands[], char *input[], char *output[], char *order[], int sizes[], char string[], char dilimeter[])
{
    //Helper function to split strings and sort the different inputs
    char *token = strtok(string, dilimeter);
    int i = 0;
    int x = 0;
    int y = 0;
    int z = 0;
    int location = 0;
    while (token != NULL)
    {
        if (i >= NUMBER_OF_STRING || x > NUMBER_OF_STRING || y > NUMBER_OF_STRING || z > NUMBER_OF_STRING)
        {
            fprintf(stderr, "Too many arguments for the Terminal\n");
            exit(EXIT_FAILURE);
        }
        int len = strlen(token);
        if (token[len - 1] == '\n')
        { // Removes newline from string like string\n
            token[len - 1] = 0;
        }
        if (!strcmp(token, "<"))
        { // If the token is < the next command is an input redirection, saves the ordering
            location = 1;
            order[z] = token;
            z++;
        }
        else if (!strcmp(token, ">"))
        { // If the token is > the next command is an output redirection, saves the ordering
            location = 2;
            order[z] = token;
            z++;
        }
        else if (location == 1)
        { //Sets the x-th input-redirection to be "token"
            input[x] = token;
            x++;
            location = 0;
        }
        else if (location == 2)
        { //Sets the y-th output-redirection to be "token"
            output[y] = token;
            y++;
            location = 0;
        }
        else
        { //Sets the i-th command to be "token"
            commands[i] = token;
            i += 1;
        }
        token = strtok(NULL, dilimeter);
    }
    commands[i] = NULL; //Ensures NULL-termination for execvp-argn[]
    sizes[0] = i;       // Amount of commands
    sizes[1] = x;       // Amount of input-redirections
    sizes[2] = y;       // Amount of output-redirections
    sizes[3] = z;       // Amount of redirections in total
}

void flusher(char *commands[], char *inputs[], char *outputs[], char *orders[], int sizes[])
{
    //Helper function to flush between commands
    for (int i = 0; i < NUMBER_OF_STRING; i++)
    {
        commands[i] = NULL;
        inputs[i] = NULL;
        outputs[i] = NULL;
        orders[i] = NULL;
    }
    for (int j = 0; j < 4; j++)
    {
        sizes[j] = 0;
    }
    fflush(stdin);
    fflush(stdout);
}

// Part 2

void exec_commands(char *commands[], int num)
{
    //Function for executing commands in child process
    pid_t pid, wait_pid;
    pid = fork();
    if (pid == 0) //Child process
    {
        int execvp_int = execvp(commands[0], commands); //Can skip redirections in part 2
        if (execvp_int == -1)
        {
            perror("The execvp failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else if (pid == -1) //The fork failes
    {
        perror("The fork failed");
        exit(EXIT_FAILURE);
    }
    while ((wait_pid == wait(NULL)) > 0) //Parent waits until child finishes
        ;
}

// Part 3

void io_redirection(char *inputs[], char *outputs[], char *orders[], int sizes[])
{
    //Function for i/o-redirection
    int input_fd, output_fd;
    int x = 0;
    int y = 0;
    for (int i = 0; i < sizes[3]; i++)
    {
        //Iterate through the redirections for input and output
        if (!strcmp(orders[i], "<"))
        {
            if ((input_fd = open(inputs[x], O_RDONLY, 0)) < 0)
            {
                // If its an input-redirection, try to open the file, dup to it, go to next redirection
                fprintf(stderr, "Failed to open %s for reading\n", inputs[x]);
                exit(EXIT_FAILURE);
            }
            // Set read from file to be handled as stdin
            dup2(input_fd, 0);
            x++;
        }
        else
        {
            if ((output_fd = creat(outputs[y], 0644)) < 0)
            {
                // If its an output-redirection, try to create or open the file, dup to it, go to next redirection
                fprintf(stderr, "Failed to open %s for writing\n", outputs[y]);
                exit(EXIT_FAILURE);
            }
            // Set write to file to be handled as stdout
            dup2(output_fd, 1);
            y++;
        }
    }
}

void exec_commands_io(char *commands[], char *inputs[], char *outputs[], char *orders[], int sizes[])
{
    //Function for executing commands in childprocess and i/o-redirection
    pid_t pid, wait_pid;
    pid = fork();
    if (pid == 0)
    {
        io_redirection(inputs, outputs, orders, sizes);
        int execvp_int = execvp(commands[0], commands); //execute commands
        if (execvp_int == -1)
        {
            perror("The execvp failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else if (pid == -1)
    {
        perror("The fork failed");
        exit(EXIT_FAILURE);
    }
    while ((wait_pid == wait(NULL)) > 0) //Parent waits until child finishes
        ;
}

// Part 4

void handle_exit(char command[])
{
    // Code for handling the internal shell command "exit"
    if (!strcmp(command, "exit"))
    {
        exit(EXIT_SUCCESS);
    }
}

int handle_cd(char directory[])
{
    // Code for handling the internal shell command "cd"
    int cd_check = chdir(directory);
    if (cd_check == -1)
    {
        perror("The cd command failed");
        exit(EXIT_FAILURE);
    }
    return 1;
}

int handle_cd_flags(char *commands[])
{
    int cd_check = 0;
    // Code for handling cd-commands using the flags "-P" and "-L"
    if ((!strcmp(commands[1], "-L") || !strcmp(commands[1], "-P")))
    {
        //Handles flags woefully inadequately by skipping them
        if (commands[2] == NULL || !strcmp(commands[2], "~"))
        {
            // This was an acceptable solution on piazza
            // Note that the cd to "HOME"-env might not be able to return to
            // mnt/c on wsl
            cd_check = handle_cd(getenv("HOME"));
        }
        else
        {
            cd_check = handle_cd(commands[2]);
        }
    }
    else
    {
        cd_check = handle_cd(commands[1]);
    }
    return cd_check;
}

int handle_internal_commands(char *commands[])
{
    // Code for handling the internal commands "exit" and "cd"
    // Redirects to the specialized functions
    int cd_check = 0;
    handle_exit(commands[0]);
    if (!strcmp(commands[0], "cd"))
    {
        if (commands[1] == NULL || !strcmp(commands[1], "~"))
        {
            cd_check = handle_cd(getenv("HOME"));
        }
        else
        {
            cd_check = handle_cd_flags(commands);
        }
    }
    // Returns whether a cd was performed or not (1=yes, 0=no)
    return cd_check;
}

void read_from_file(char file[])
{
    pid_t pid, wait_pid;
    int input_fd;
    int internal = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *commands[NUMBER_OF_STRING];
    char *inputs[NUMBER_OF_STRING];
    char *outputs[NUMBER_OF_STRING];
    char *orders[NUMBER_OF_STRING];
    int sizes[4];
    pid = fork();
    if (pid == 0)
    {
        if ((input_fd = open(file, O_RDONLY, 0)) < 0)
        {
            // If its an input-redirection, try to open the file, dup to it, go to next redirection
            fprintf(stderr, "Failed to open %s for reading\n", file);
            exit(EXIT_FAILURE);
        }
        // Set read from file to be handled as stdin
        dup2(input_fd, 0);
        while ((nread = getline(&line, &len, stdin)) != -1)
        {
            split_string(commands, inputs, outputs, orders, sizes, line, " ");
            internal = 0;
            internal = handle_internal_commands(commands);
            if (!internal)
            {
                exec_commands_io(commands, inputs, outputs, orders, sizes);
            }
        }
        free(line);
        fclose(stdin);
        exit(EXIT_SUCCESS);
    }
    while ((wait_pid == wait(NULL)) > 0) //Parent waits until child finishes
        ;
}

void part_1(char *commands[], char *input[], char *output[], char *order[], int sizes[])
{
    //Function for printing out the commands and directions
    int x = 0;
    int y = 0;
    for (int i = 0; i < sizes[0]; i++)
    {
        if (i == 0)
        {
            printf("Command name: %s \n", commands[0]);
            printf("Command parameters: ");
        }
        else
        {
            printf("%s", commands[i]);
            if ((i + 1) != sizes[0])
            {
                printf(", ");
            }
            else
            {
                printf("\n");
            }
        }
    }
    printf("Redirections: ");
    for (int z = 0; z < sizes[3]; z++)
    {
        if (!strcmp(order[z], "<"))
        {
            printf("%s", input[x]);
            x++;
        }
        else
        {
            printf("%s", output[y]);
            y++;
        }
        if ((z + 1 != sizes[3]))
        {
            printf(", ");
        }
    }
    printf("\n");
}

void part_2(char *commands[], int num)
{
    exec_commands(commands, num);
}

void part_3(char *commands[], char *inputs[], char *outputs[], char *orders[], int sizes[])
{
    exec_commands_io(commands, inputs, outputs, orders, sizes);
}

void part_4(char *commands[], char *inputs[], char *outputs[], char *orders[], int sizes[])
{
    int internal = 0;
    internal = handle_internal_commands(commands);
    if (!internal)
    {
        exec_commands_io(commands, inputs, outputs, orders, sizes);
    }
}

void part_5(char file[])
{
    read_from_file(file);
}

int main(int argc, char **argv)
{
    char input[MAX_STRING_SIZE];
    char dilimeter[2];
    char *commands[NUMBER_OF_STRING];
    char *input_redirection[NUMBER_OF_STRING];
    char *output_redirection[NUMBER_OF_STRING];
    char *order[NUMBER_OF_STRING];
    int sizes[4];
    int wrong_input = 1;
#if PART5
    if (argc == 1)
    {
#endif
        while (wrong_input)
        {
            printf("Do you prefer tab or space as dilimeter? (t/s) \n> ");
            if (fgets(dilimeter, sizeof(dilimeter), stdin) == NULL)
            {
                printf("Input error.\n");
                exit(1);
            }

            if (!strcmp(dilimeter, "t"))
            {
                FLUSH_STDIN(dilimeter);
                strcpy(dilimeter, "\t");
                wrong_input = 0;
            }
            else if (!strcmp(dilimeter, "s"))
            {
                FLUSH_STDIN(dilimeter);
                strcpy(dilimeter, " ");
                wrong_input = 0;
            }
            else
            {
                printf("Wrong input should be 't' or 's'! \n");
            }
        }

        while (1)
        {
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            printf("%s> ", cwd);
            //TODO: should fgets be getline?
            flusher(commands, input_redirection, output_redirection, order, sizes); //set all fields to NULL, ensures NULL-termination
            fgets(input, sizeof(input), stdin);                                     // read string
            //printf("Input: ");
            //puts(input); // display string
            split_string(commands, input_redirection, output_redirection, order, sizes, input, dilimeter);

#if PART1
            part_1(commands, input_redirection, output_redirection, order, sizes);
#endif
#if PART2
            part_2(commands, sizes[0]);
#endif
#if PART3
            part_3(commands, input_redirection, output_redirection, order, sizes);
#endif
#if (PART4 || PART5)
            part_4(commands, input_redirection, output_redirection, order, sizes);
#endif
        }
#if PART5
    }
    part_5(argv[1]);
#endif
    return 0;
}
