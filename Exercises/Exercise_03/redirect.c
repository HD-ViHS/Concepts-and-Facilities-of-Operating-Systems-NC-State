#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

static void fail( char *msg ) {
    // This program does redirection of standard output.  Good thing
    // we still have standard error around, so we can print error
    // messages even 
    fprintf( stderr, "Error: %s\n", msg );
    exit( EXIT_FAILURE );
}

/**
 * @brief The redirect.c program redirects input from standard input and standard output
 *        and instead writes to "input.txt" and "output.txt" respectively. The cat command
 *        is then run with input and output from the files instead of the input and output
 *        streams.
 * 
 * @file redirect.c
 * @author Siobhan McCarthy
 */
int main( int argc, char *argv[] ) {
    // Make a child process to run cat.
    pid_t pid = fork();
    if ( pid == -1 )
        fail( "Can't create child process" );

    if ( pid == 0 ) {
        // I'm the child.  Before replacing myself with the cat program,
        // change its environment so it reads from "input.txt" instead of
        // standard input.

        int inputHandle = open("input.txt", O_RDONLY);
        dup2(inputHandle, STDIN_FILENO);
        close(inputHandle);

        // ... and writes to "output.txt" instead of standard output.

        int outputHandle = open("output.txt", O_WRONLY | O_CREAT, 00700);
        dup2(outputHandle, STDOUT_FILENO);
        close(outputHandle);

        // Now, run cat.  Even though it thinks it's reading from standard
        // input and writing to standard output, it will really be copying these
        // files.

        execl("/bin/cat", "cat", (char *) NULL);
        fail("Could not run cat command");
    }

    // Wait for the cat program to finish.
    wait( NULL );

    return EXIT_SUCCESS;
}
