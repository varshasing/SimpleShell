#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 512                                        // maximum number of lines from command line. use the size for malloc
#define MAX_LENGTH_ARGS 32                                  // maximum number of letters in an argument
#define TRUE 1


// struct definition. Stores commands, arguments, flag for background, in/out redirection, and LL fashion for next command in a pipeline
// used for string parser

// run commands in standard way in function with pipes and checks to see if it is a background process and if it needs redirection in special cases
// return child PID for waitpid in main
// free memory for the_command struct(s) in main
// int isFirst isLast, bool redirection flags
void sighand()
{
    pid_t pid;
    int status;
    while(TRUE)
    {
        pid = waitpid(-1, &status, WNOHANG);
        if(pid <= 0)
        {
            break;
        }
    }
    return;
}
int run_command(the_command* command, int input_fd, int isFirst, int isLast) {
    pid_t pid;
    int status;
    int pipe_fd[2];     // Used for piping between commands
    int in_fd, out_fd;  // Separate file descriptors for input/output redirection

    // If not the last command, create a pipe for passing output to the next command
    if (!isLast) {
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Fork the process
    if ((pid = fork()) == 0) {  // Child process

        // Handle output redirection if specified
        if (command->redirect_output) {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (out_fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);  // Redirect stdout to file
            close(out_fd);
        } else if (!isLast) {
            // If it's not the last command, redirect output to the pipe
            dup2(pipe_fd[1], STDOUT_FILENO);
        }

        // Handle input redirection if specified
        if (command->redirect_input) {
            in_fd = open(command->redirect_input, O_RDONLY);
            if (in_fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);  // Redirect stdin to file
            close(in_fd);
        } else if (!isFirst) {
            // If it's not the first command, input comes from the previous pipe
            dup2(input_fd, STDIN_FILENO);
        }

        // Execute the command
        if (execvp(command->command, command->arguments) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

    } else {  // Parent process

        if (!command->is_background) {
            waitpid(pid, &status, 0);  // Wait for child process to finish
        }

        // Close the write end of the pipe in the parent process
        if (!isLast) {
            close(pipe_fd[1]);
        }

        // Close input_fd from previous pipe if it’s not stdin
        if (input_fd != STDIN_FILENO) {
            close(input_fd);
        }

        // Return the read end of the pipe for the next command in the pipeline
        return isLast ? 0 : pipe_fd[0];
    }

    return 0;
}




int main(int argc, char* argv[])
{
    char* buffer;                                           // buffer to store the input from stdin
    buffer = malloc(sizeof(char)*MAX_LINE);                 // allocate memory for the buffer
    if(!buffer)
    {
        exit(EXIT_FAILURE);
    }
    char* token_array[MAX_LINE];
    int suppress = 0;
    if(argc > 1)
    {
        if(strcmp(argv[1], "-n") == 0)                      // if the -n flag is used for grading, suppress the my_shell$
        {
            suppress = 1;
        }
    }
    signal(SIGCHLD, sighand);

    while(TRUE)                                             // REPL: read, evaluate, print, loop; goes until CTRL+D ends it
    {
        if(!suppress)
        {
            printf("my_shell$ ");                           // print the shell prompt
            fflush(stdout);
        }
        if(fgets(buffer, MAX_LINE, stdin) == NULL)          // get the input from the command line
        {
            if(feof(stdin))
            {
                break;
            }
        }
        int len = strlen(buffer);
        if(len > 0 && buffer[len - 1] == '\n')              // remove the newline character from the buffer
        {
            buffer[len - 1] = '\0';
        }
        int i;
        for(i = 0; i < MAX_LINE; i++)                       // initialize the token array to NULL, resets each time
        {
            token_array[i] = NULL;
        }
        //i = 0;
        tokenize(buffer, token_array);                      // tokenize the buffer into an array of c-strings
        the_command* head = parse_buffer(token_array);      // creates linked list of commands
        the_command* current = head;                        // first command in LL format

        int input_fd = STDIN_FILENO;                        // input fd
        int output_fd = STDOUT_FILENO;                      // output fd
        while(current->next_command != NULL)                // for each command struct
        {
            input_fd = run_command(current, input_fd, (current == head), (current->next_command == NULL));     // run the command
            current = current->next_command;                // move to the next command
        }

        input_fd = run_command(current, input_fd, (current == head), (current->next_command == NULL));     // run the command

        free_struct(head);
        fflush(stdout);
    }
    free(buffer);
    return 0;
}


/*
    while(TRUE)
    {
    type_prompt();
    read_command(command, parameters);
    if(fork() != 0)
    // parent code
        waitpid(-1, &status, 0);
    else
    {
    // child code
        execve(command, parameters, 0);
    }
*/


/*
I am getting confused with connecting pipes bc hpw dpes the source sink thing even work :tomatoes:
*/

/*

if(command->redirect_input == NULL)     // set to STDIN if there is no inout redirection expected
        {
            in_fd = STDIN_FILENO;
        }
        else
        {
            in_fd = open(command->redirect_input, O_RDONLY);         // open the file for reading
            if(in_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        if(command->redirect_output == NULL)    // set to STDOUT if there is no output redirection expected
        {
            out_fd = STDOUT_FILENO;
        }
        else
        {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);     // open the file for writing
            if(out_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
        }
        if(in_fd != STDIN_FILENO)
        {
            if((pipe_fd[1] = open(command->redirect_input, O_RDONLY)) != 0)         // stdin reads FROM in
            {
                perror("IN_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(in_fd);
        }
        if(out_fd != STDOUT_FILENO)
        {
            if(dup2(out_fd, STDOUT_FILENO) == -1)       // stdout writes TO out
            {
                perror("OUT_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }
        //execvp(command->arguments[0], command->arguments);     // execute the command
        if(!isLast)                                     // if this is not the last command in pipe, need to util pipe logic
        {
            if(dup2(pipe_fd[1], STDOUT_FILENO) == -1)   // output goes to the write end of the pipe
            {
                perror("pipe1 DUP2");
                exit(EXIT_FAILURE);
            }
            close(pipe_fd[1]);
        }
        if(command->redirect_output != NULL && isLast)  // output redirection for last command, assumption from doc
        {
            out_fd = open(command->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if(out_fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            if(dup2(out_fd, STDOUT_FILENO) == -1)
            {
                perror("OUT_FD DUP2");
                exit(EXIT_FAILURE);
            }
            close(out_fd);
        }


    }*/