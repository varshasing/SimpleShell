## Notice
This repository is for educational purposes only. Forking, copying, or redistributing this code is not allowed without explicit permission.

# Operating Systems: Varsha Singh
#### For the first (mini-deadline),  you shell must demonstrate that it can accept user-input, parse that input, and invoke simple commands with arguments. That is, you will need to implement the parsing logic, and at least steps 1, 2, and 4 from the "some hints" section. In order to test your shell-program, it must REPL and for each command-line output the following:
```c
<first command>
<second command>
...
<nth command>
whatever execvp() of the first command produces
```
#### For illustration purposes consider the following example:
```c
/bin/echo foo | /bin/grep -o m 1 f | /user/bin/wc -c > /tmp/x
```
### Your program would parse this command line into 3 different commands (technically, the last argument is a file) and output (Line numbers for illustration only).
```c
1: /bin/echo foo
2: /bin/grep -o -m 1 f
3: /user/bin/ec -c > /tmp/x
4: foo
```
#### Above, lines 1-4 are the three commands making up the command-line, line 4 (the string foo) is the result of the first command (/bin/echo foo).

## Approach
To meet the mini- deadline, the following needed to be implemented:
1. A command-line parser to figure out what the user is trying to do.
2. If a valid command is entered, the shell should ```c fork(2)``` to create a new (child) process, and the child process should ```c exec``` the command.
3. Split apart the command(s) to separate the command line into different commands via piping,
4. Prepare for the user to run the executable file with -n as an argument to suppress the ```c my_shell$ ``` command-line prompt.
5. Utilize ```c CTRL+D``` to terminate the shell program.

With these steps, I outlined a simple procedure:
### __Implement a String Parser__
The shell parser must take in the user inputs and *tokenize* the command-line input.
In *Lecture 0*, we discussed an approach to a string parser by using the ```c strpbrk(3)``` function, which is called via ```c char* strpbrk(const char *s, const char *accept)``` The ```c strpbrk(3)``` function locates the first occurence in the string s of any of the bytes in the string accept, returning a pointer to the bute in s that matches one of the bytes in accept, or NULL if no such byte is found.

I found this method to be a good starting point, but it introduced very erroneous heuristic approaches that would fail to correctly parse with the caveat in special characters. The caveat is that spaces aren't neccessary between special characters, so my first implementation struggled with this function.

This leads to my implemented approach: using dynamic memory allocation to fill in the ```c char* token_array[]``` with an implementation of a fast and slow pointer.
#### __Fast and Slow Pointers__
Fast and Slow pointers is a coding pattern that uses two pointeers to traverse a data structure at different speeds, and is also known as the Hare and Tortoise algorithm. They are usually used with arrays and linked lists, and is a methodical approach I learned about in *ENG EC330: Algorithms and Data Structures for Engineers* as well as when solving *Leetcode* questions for interview preparation.

For tokenizing based on each individual parameter:
__While__ loop which utilized the fast pointer to move across the given *buffer* command-line input. While the fast pointer was not at the end of the buffer (noted by the pointer not equaling the null-terminator), go through the array until it encounters a non-space. This indicates a new arguement. Once this occurs, move the slow pointer to the same address.
__If__ the fast pointer is pointing to one of the special characters, it must be added to the token list and we move on from this special character. Because the *tokenize* function is dynamically filling in the tokens into the *token_array*, this meant I must dynamically allocate memory for each token, else I would lose the values once the tokenize function is exited.
Thus, dynamically allocate memory into the *token_array* index of size 2 bytes, the first byte is the special character, and the second byte is the null terminator. I increment the fast pointer to the next address, and resynchronize the slow and fast pointer. 
__Else,__ while the fast pointer is not pointing to a special character, space, or a null terminator, continue to increment it. This means that we are trying to get past an argument. Once this condition is broken, I use pointer arithmentic, to see if the difference between the fast pointer and slow pointer is greater than zero. If so, then I have successfully moved onto another argument that must be tokenized, and allocate the space in the *token_array* index, and calliong ```c strncpy()``` to copy the characters into the allocated space, and assigning a null terminator to the end, because it is more often a space or an adjacent special character. Then, resynchronize the slow pointer to the fast pointer, and increment the *token_index*.

This process goes through the entire loops until the fast pointer is pointing to a null terminator, in which it will do the last tokenization.

### __Parsing the Buffer__
Because I had utilized fast and slow pointers for the tokenizer, it made me think of using struct in a fashion of a Linked List:
#### __Linked List__
To prepare myself a little bit more for the final deadline, I had to think of an idea for being able to free memory of processes, as well as running piped processes. This lead to the idea of a linked list, that can point to another command it is piped with. This way, I am able to go through each struct, use fork() and execvp(), and move onto the next one in the pipe.
To do this, I created the struct, ```c the_command```, which has the following:
```c  char* command;
    char** arguments;

    int is_background;
    char* redirect_input;
    char* redirect_output;

    struct the_command* next_command;
```
With this, I had to separate the structs via piping, making sure to take the first word as the command and putting every single argument in the arguments of the structure.

Because we are only executing the first command for the the mini-deadline, I coded it such that it would always only run the first command's values with execvp. To print out each of the parsed commands, I initiated a while loop for the pointer to structs to not be null, and prinited out all of the commands on one line.

I found slight difficulty in allocating memory properly, but solved it with the man pages and by writing out simple examples by hand to figure out hwo much memory to allocate. As based on my test inputs, I was able to create a functional shell for the mini-deadline, but was not able to pass the autograder, to which a classamte suggesting using fflush for stdout after each print statement. This, along with fixing issues like:
1. After adding my special character to the arguments, I had logic which accidentally put the special character in my array twice, resulting in wrong printing of the arguments
2. using strlcpy on Bandit, but strlcpy was not recognized during compilation with ```c gcc -Werror -o myshell main.c parser.c```. To fix this, I changed all strlcpy to strcpy, except for in one instance in which I knew the length to copy would always be 1 (special characters).
