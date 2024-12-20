#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

// This should work for the mini-deadline, which only takes in one command and its arguments, and no special characters.
// the setup should work for the final deadline, but I won't implement the logic for the final deadline in main right now.
// assumption, no leading or trailing spaces
#define MAX_LINE 512
#define MAX_ARG_LENGTH 32

#define char_delimit " <>|"                                                     // special characters that will be used for determinig if command struct needs to be created

// writing struct commands

void tokenize(char* buffer, char* token_array[])
{
    // go through the buffer and tokenize it based on each individual parameter
    // want to return an array of c-strings

    if(buffer == NULL)                                                          // if the buffer is empty, don't parse anything
    {
        return;
    }

    char* slow = buffer;                                                        // slow pointer to the buffer. Will be at start of words (expect for special characters with no spaces)
    char* fast = buffer;                                                        // fast pointer to the buffer
    int token_index = 0;                                                        // index for the token array
                                                                                // can use pointer arithmatic to dynamically allocate memory for each token
    while(*fast != '\0')
    {
        while(*fast != '\0')
        {
            if(*fast == ' ')                                                    // iterate the fast pointer until it reaches the start of the next word
            {
                fast++;
            }
            else
            {
                break;
            }
        }
        slow = fast;                                                            // realign
        if( *fast == '<' || *fast == '>' || *fast == '|' || *fast == '&')       // if special character is found, make sure to add it to the token list and move on
        {
            token_array[token_index] = malloc(sizeof(char)*2);
            strncpy(token_array[token_index], fast, 1);
            token_array[token_index][1] = '\0';
            token_index++;                                                      // number of tokens increase
            fast++;
            slow = fast;                                                        // realign
        }
        else{
            while(*fast != ' ' && *fast != '<' && *fast != '>' && *fast != '|' && *fast != '&' && *fast != '\0')        // while fast is still on an argument that isn't a special character
            {
                fast++;
            }
            if((long)(fast - slow) > 0)                                         // pointer arithmatic, if there is a token to be added
            {
                token_array[token_index] = malloc(sizeof(char) * ((fast - slow)+1));        // allocate memory for the token
                strncpy(token_array[token_index], slow, (long)(fast - slow));
                token_array[token_index][(long)(fast - slow)] = '\0';
                token_index++;
                slow = fast;                                                    // realign
            }
        }
    }
    token_array[token_index] = NULL;                                            // set the last token to NULL

}

// need to add functionality for showing that & make all tasks background tasks in that line, as well as redirecting in/output
// can do that by changing logic for the is_background flag, going through all tasks and making them operate in background (look this up??)

// splits the tokenized command line array into structs, each struct representing a command
// WIP: do not add redirection and place to be redirected to into the arguments, just the command
the_command* parse_buffer(char* token_array[])                                  // create a linked structs for each command
{
    the_command* head = malloc(sizeof(the_command));
    if(!head)                                                                   // if isn't blank, then there is some command to be put in struct
    {
        perror("malloc head");
        return NULL;
    }
    the_command* current = head;                                                // current pointer to the head of the linked list
    current->is_background = 0;                                                 // initialize the background flag to 0. IF & at end, set all is_background to &
    current->next_command = NULL;                                               // initialize the next command to NULL
    current->redirect_input = NULL;                                             // initialize the input redirection to NULL
    current->redirect_output = NULL;                                            // initialize the output redirection to NULL

    int command_end = 0;                                                        // used for finding the end of an argument
    int command_start = 0;                                                      // used for finding the start of an argument
    int token_index;                                                            // used to keep count of number of arguments in one command

    while(token_array[command_end] != NULL)
    {
        int token_count = 0;                                                    // initalize to zero for each command
        token_index = 0;                                                        // initalize to zero for each command
        while(token_array[command_end] != NULL && (strcmp(token_array[command_end], "|") != 0))     // count number of arguments for structs arguments
        {
            token_count++;
            token_index++;
            command_end++;
        }
        current->command = malloc(sizeof(char)*(strlen(token_array[command_start]) + 1));       // allocate memory and store command
        if(!current->command)                                                   // check for malloc failure
        {
            perror("malloc command");
            return NULL;
        }
        strcpy(current->command, token_array[command_start]);                   // ASSUME: command is always first in the array, before/after pipe
        current->arguments = malloc(sizeof(char*)*(token_index+1));
        int i;
        // WIP, don't want the redirection or file for redirection to be included, should change logic for putting into arguments
        // example: ls -l > file.txt, should be ls -l, file.txt should be in redirect_output
        // example: cat < x > y, should be cat; x should be in redirect_input, y should be in redirect_output
        for(i = 0; i < token_count; i++)                                        // allocate memory and store all arguments in a command, including command binary. need to take out x and y from arguments
        {
            if(strcmp(token_array[command_start + i], "<") == 0 && token_array[command_start + i + 1] != NULL)      // if input redirection, store the input redirection in proper struct loc, move past to avoid keeping in args
            {
                current->redirect_input = malloc(sizeof(char)*(strlen(token_array[command_start + i + 1]) + 1));
                strcpy(current->redirect_input, token_array[command_start + i + 1]);
                i += 1;
            }
            else if(strcmp(token_array[command_start + i], ">") == 0 && token_array[command_start + i + 1] != NULL)     // if output redirection, store the output redirection in proper struct loc, move past to avoid keeping in args
            {
                current->redirect_output = malloc(sizeof(char)*(strlen(token_array[command_start + i + 1]) + 1));
                strcpy(current->redirect_output, token_array[command_start + i + 1]);
                i += 1;
            }
            else if(strcmp(token_array[command_start + i], "&") == 0)                                                   // if background process, set ALL flags to 1
            {                                                                                                           // ASSUMPTION: & can only appear once and at the end of the line
                the_command* temp = head;
                while(temp != NULL)
                {
                    temp->is_background = 1;
                    temp = temp->next_command;
                }
            }
            else                                                                                                        // if normal argument, store the argument in proper struct loc
            {
                current->arguments[i] = malloc(sizeof(char)*(strlen(token_array[command_start + i]) + 1));
                strcpy(current->arguments[i], token_array[command_start + i]);
            }
        }
        current->arguments[token_count] = NULL;                                 // set the last argument to NULL for execvp

        command_start = command_end + 1;                                        // move the start to the next command
        if(token_array[command_end] != NULL && strcmp(token_array[command_end], "|") == 0)      // if there is a piped command, create a new struct
        {
            current->next_command = malloc(sizeof(the_command));
            current = current->next_command;
            current->is_background = 0;
            current->next_command = NULL;
            current->redirect_input = NULL;
            current->redirect_output = NULL;
            command_end++;
        }
    }
    return head;
}

void free_struct(the_command* head)                                            // free the memory allocated for the structs after command-line input is done
{
    the_command* temp;
    while(head != NULL)
    {
        temp = head;
        head = head->next_command;
        free(temp->command);
        int i = 0;
        while(temp->arguments[i] != NULL)
        {
            free(temp->arguments[i]);
            i++;
        }
        free(temp->arguments);
        if(temp->redirect_input != NULL)
        {
            free(temp->redirect_input);
        }
        if(temp->redirect_output != NULL)
        {
            free(temp->redirect_output);
        }
        free(temp);
    }
}