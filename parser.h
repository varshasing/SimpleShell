#ifndef PARSER_H
#define PARSER_H

#define MAX_LINE 512
#define MAX_ARG_LENGTH 32
#define MAX_ARGS MAX_LINE/2 + 1
#define TRUE 1

typedef struct the_command
{
    char* command;                                      // c-string for command
    char** arguments;                                   // c-string for pointers to each argument, including command for execvp

    int is_background;                                  // flag for background process
    char* redirect_input;                               // c-string for input redirection
    char* redirect_output;                              // c-string for output redirection

    struct the_command* next_command;                   // pointer to the next command, NULL subsequent command via pipeline
} the_command;

the_command* parse_buffer(char* token_array[]);         // function to parse the buffer into the_command struct(s)
void free_struct(the_command* command);                 // function to free the memory allocated for the_command struct(s)
void tokenize(char* buffer, char* token_array[]);       // function to tokenize the buffer into all arguments


#endif