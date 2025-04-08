#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief A generic failure function for error handling.
 * 
 * @param msg the error message to print
 */
static void fail( char *msg ) {
    printf( "Error: %s\n", msg );
    exit( EXIT_FAILURE );
}

/**
 * @brief The children.c program creates 3 children, and prints their process IDs and parent process IDs.
 * 
 * @file children.c
 * @author Aidan McCarthy amccart4
 */
int main( int argc, char *argv[] ) {
    // Try to make a child process.
    int pid = fork();
    if ( pid == -1 )
        fail( "Can't create child process" );

    // Print out a report from that child.
    if ( pid == 0 )
    {
        sleep( 1 ); // Wait for a second
        printf( "I am %d, child of %d\n", getpid(), getppid() );
        exit( EXIT_SUCCESS );
    }
    else // Running the parent process
    {
        int pid_2 = fork(); // Try to make a second child process.
        if ( pid == -1 )
            fail( "Can't create child process" );
        if ( pid_2 == 0)
        {
            sleep( 1 ); // Wait for a second
            printf( "I am %d, child of %d\n", getpid(), getppid() );
            exit( EXIT_SUCCESS );
        }
        else // Running the parent process
        {
            int pid_3 = fork(); // Try to make a third child process
            if ( pid == -1 )
                fail( "Can't create child process" );
            if ( pid_3 == 0)
            {
                sleep( 1 ); // Wait for a second
                printf( "I am %d, child of %d\n", getpid(), getppid() );
                exit( EXIT_SUCCESS );
            }
        }
    }
    wait(NULL); // Wait for child 1
    wait(NULL); // Wait for child 2
    wait(NULL); // Wait for child 3
    printf( "Done\n" );
    return EXIT_SUCCESS;
}