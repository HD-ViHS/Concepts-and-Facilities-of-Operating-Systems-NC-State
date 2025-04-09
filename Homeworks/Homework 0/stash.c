#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_WORDS 513 // The maximum number of words that can come from a 1024 char input line
#define ASCII_NINE 57
#define ASCII_ZERO 48
#define MAX_LINE_LENGTH 1024 // Maximum number of characters allowed on a command line
/**
 * @brief The parseCommand function takes a line of user input and parses it using an array of char
 *        pointers and null termination in the input line to separate out individual tokens. Returns
 *        -1 if there are too many characters in the input line (error code).
 * 
 * @param line the command line to parse.
 * @param words the array of char pointers to point to tokens in the command line.
 * @return int the number of words parsed from the command, -1 on too large an input.
 */
int parseCommand( char *line, char *words[] )
{
    int length = strlen(line);
    if (length > 1024)
        return -1;

    bool readingWord = false; // Are we currently reading a word?
    int wordCount = 0;
    for(int i = 0; i < length; i++)
    {
        if (isspace(line[i]) == 0) // Not a whitespace Character
        {
            readingWord = true; // We are reading a word
            if(i == 0) // Handle case of no leading whitespace before first command
            {
	            words[wordCount] = ( line + i ); // Set words pointer to first command
                wordCount++; // Increment word count
            }
            else // Any other case besides first character
            {
	            if((isspace(line[i - 1]) != 0) || line[i - 1] == '\0') // Safe because we already handled the i == 0 case, current letter is not whitespace but previous letter is (or is previously modified null char
                {
		            words[wordCount] = ( line + i ); // set words pointer to current token
                    wordCount++; // Increment word count
                }
            }
        }
        else if (readingWord) // Reached a whitespace character immediately after reading a word
        {
            readingWord = false; // Now reading whitespace
            line[i] = '\0'; // Null terminate the command
        }
    }
    return wordCount;

}

/**
 * @brief The runExit program exits the stash program with the given status code.
 *        The program does not exit if the command was incorrectly entered (too many or
 *        too few commands, second command no parseable as an int)
 * 
 * @param words the list of pointers to words in the user's command
 * @param count the number of words in the words array
 */
void runExit( char *words[], int count )
{
    if (count != 2) // Incorrect number of tokens
        printf("Invalid command\n");

    else
    {
        int digitLength = strlen(words[1]); // Number of digits in the exit code
        int parsedInt = 0;
        int placeValue = 1;
        for(int i = digitLength - 1; i >= 0; i--) // Parese the exit code as an int
        {
            int digit = words[1][i];
            if (digit > ASCII_NINE || digit < ASCII_ZERO)
            {
                printf("Invalid command\n");
                return;
            }
            parsedInt += ((digit - ASCII_ZERO) * placeValue);
            placeValue *= 10;
        }
        exit(parsedInt); // Exit the program with the exit code being the parsed int
    }
}

/**
 * @brief The runCd command runs the change directory command. No directory is changed if an
 *        incorrect number of arguments (!= 2) is given or if the directory could not
 *        be changed with the use of the chdir() system call
 * 
 * @param words the list of tokens in the command line
 * @param count the number of words in the words list
 */
void runCd( char *words[], int count )
{
    if(count != 2) // Incorrect number of arguments
        printf("Invalid command\n");

    else if(chdir(words[1]) < 0) // Couldn't change directory (chdir() returns -1 on an error)
        printf("Invalid command\n");
}

/**
 * @brief The runCommand function runs command-line commands given to the stash.
 *        A child is forked to run the process and is given the execvp() command
 *        on the words pointer array to run the command given.
 * 
 * @param words the array of char pointers to command line arguments
 * @param count the number of tokens in the command line
 */
void runCommand( char *words[], int count )
{
    words[count] = NULL;
    pid_t pid = fork(); // Fork the child
    if (pid == -1 ) // Fork failed
    {
        printf("Can't run command %s\n", words[0]); // Error Message
        return;
    }
    if(pid == 0) // Find the child
    {
        if(execvp(words[0], words) == -1) // Execute command and error check
        {
            printf("Can't run command %s\n", words[0]); // Error message
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(NULL); // Wait for child to complete
    }
}

/**
 * @brief The stash.c program runs a comman-line interface shell named stash
 *        (Simple Toy Assessment SHell). The shell supports three commands
 *        (cd, exit, and a blank line). Additionally, the shell supports
 *        commands runnable via the execvp() system call.
 * 
 * @file stash.c
 * @author Siobhan McCarthy
 */
int main(int argc, char *argv[])
{
    while(true) // Run until exit
    {
        printf("stash> "); // Beginning prompt
        char line[MAX_LINE_LENGTH]; // Input line
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) // Returning NULL
        {
            printf("\n");
            continue;
        }
        char *words[MAX_WORDS];
        int numWords = parseCommand(line, words);
        if (numWords == -1)
        {
            printf("Too many characters in the command line\n"); // A -1 on parsing means the line length is > 1024
            continue;
        }
	    else if (numWords == 0)
	        continue;

        if(strcmp(words[0], "cd") == 0)
            runCd(words, numWords);

        else if(strcmp(words[0], "exit") == 0)
            runExit(words, numWords);

        else
            runCommand(words, numWords);
    }
}