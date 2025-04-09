#include <stdio.h>
#include <unistd.h>    // for write
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Define the start routines for your three threads.  Each thread will
// use two calls to write() to print its two characters, then it will
// terminate.

/**
 * @brief The start routine for the first thread.
 * 
 * @param arg The void pointer to the arguments for the start routine
 * @return void* routine should return null
 */
void *thread1Start( void *arg ) {
  write( STDOUT_FILENO, "a", 1 );
  write( STDOUT_FILENO, "b", 1 );
  return NULL;
}

/**
 * @brief The start routine for the first thread.
 * 
 * @param arg The void pointer to the arguments for the start routine
 * @return void* routine should return null
 */
void *thread2Start( void *arg ) {
  write( STDOUT_FILENO, "c", 1 );
  write( STDOUT_FILENO, "d", 1 );
  return NULL;
}

/**
 * @brief The start routine for the first thread.
 * 
 * @param arg The void pointer to the arguments for the start routine
 * @return void* routine should return null
 */
void *thread3Start( void *arg ) {
  write( STDOUT_FILENO, "e", 1 );
  write( STDOUT_FILENO, "f", 1 );
  return NULL;
}

/**
 * @brief The interleaving.c program created three threads, assigns each to 
 *        a start routine (above), runs them, to which "a", "b", 
 *        "c", "d", "e", and "f" should be printed out in whatever order
 *        the scheduler gets to them.
 * 
 * @author Siobhan McCarthy
 * @file interleaving.c
 * @return int the exit status of the program
 */
int main( int argc, char *argv[] ) {
  // A bunch of times.
  for ( int i = 0; i < 100000; i++ ) {
    // Make three threads.

    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    if ( pthread_create(&thread1, NULL, thread1Start, NULL ) != 0 )
      fail( "Can't create child thread 1" );

    if ( pthread_create(&thread2, NULL, thread2Start, NULL ) != 0 )
      fail( "Can't create child thread 2" );

    if ( pthread_create(&thread3, NULL, thread3Start, NULL ) != 0 )
      fail( "Can't create child thread 3" );
    
    // Join with the three threads.

    pthread_join( thread1, NULL );
    pthread_join( thread2, NULL );
    pthread_join( thread3, NULL );

    // Use the write system call to print out a newline.  The string
    // we're passing to write is null terminated (since that's what
    // double quotesd strings are in C), but we're just using the
    // first byte (the newline).  Write doesn't care about null
    // terminated strings, it just writes out any sequence of bytes
    // you ask it to.
    write( STDOUT_FILENO, "\n", 1 );
  }

  return 0;
}
