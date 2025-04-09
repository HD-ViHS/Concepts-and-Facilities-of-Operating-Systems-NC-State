#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief A general fail function for the program.
 * 
 * @param msg the failure message to print to stderr
 */
static void fail( char *msg ) {
    fprintf( stderr, "Error: %s\n", msg );
    exit( EXIT_FAILURE );
}

/**
 * @brief The pipe.c program creates 2 children and a pipe, the purpose of which
 *        is to run the command line command "ps -ef | grep bash".
 * 
 * @file pipe.c
 * @author Siobhan McCarthy
 * @return int the return status of the program
 */
int main( int argc, char *argv[] ) {
    // Make a pipe for our two children to talk over.
    int pfd[2];

    if( pipe( pfd ) != 0)
        fail("Failure creating pipe.");

    // Make a child process to run ps -ef.
    pid_t pid = fork();
    if ( pid == -1 )
        fail( "Can't create child process" );

    if ( pid == 0 ) {
        // I'm the child.  Before replacing myself with the ps program,
        // change my environment.  First, close the reading end of the
        // pipe, since I won't need it.

        if ( close(pfd[0]) != 0 )
            fail("Cannot close reading end of pipe in first child.");

        // Then, replace my standard output with the writing end of the pipe.
    
        if ( dup2(pfd[1], STDOUT_FILENO) == -1 )
            fail("Cannot replace stdout with writing end of pipe in first child.");

        // I just made a copy of the writing end of the pipe, close the old copy
        // of the writing end

        if ( close(pfd[1]) != 0 )
            fail("Cannot close writing end of pipe in first child.");

        // Now, execl ps -ef

        execl("/bin/ps", "/bin/ps", "-ef", (char *) NULL);

        fail( "Can't run ps -ef" );
    }

    // Make another child process to run grep bash
    pid = fork();

    if ( pid == -1 )
        fail( "Can't create second child process" );

    if ( pid == 0 ) {
        // I'm the child.  Before replacing myself with the ps program,
        // change my environment.  First, close the wriing end of the
        // pipe, since I won't need it.

        if ( close(pfd[1]) != 0 )
            fail("Cannot close writing end of pipe in second child.");

        // Then, replace my standard input with the reading end of the pipe.

        if ( dup2(pfd[0], STDIN_FILENO) == -1 )
            fail("Cannot replace stdin with reading end of pipe in second child.");

        // I just made a copy of the reading end of the pipe, close the old copy
        // of the reading end.

        if ( close(pfd[0]) != 0 )
            fail("Cannot close reading end of pipe in second child.");


        // Now, execl grep bash

        execl("/bin/grep", "/bin/grep", "bash", (char *) NULL);

        fail( "Can't run grep bash" );
    }

    // I'm the parent process.  I don't need either end of the pipe, it's just
    // for childrent to communicate.

    if ( close(pfd[0]) != 0 )
        fail("Cannot close reading end of pipe in parent.");

    if ( close(pfd[1]) != 0 )
        fail("Cannot close writing end of pipe in parent.");

    // Wait for both of the children to terminate.
    wait( NULL );
    wait( NULL );

    return EXIT_SUCCESS;
}