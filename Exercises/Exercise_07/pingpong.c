#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Number of iterations for each thread
#define ITERATIONS 500

// Who gets to go next.
int nextTurn = 0;

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Start routines for the two threads.

void *ping( void *arg ) {
  for ( int i = 0; i < ITERATIONS; i++ ) {
    while ( nextTurn != 0 )
      ;

    nextTurn = 1;
  }
  return NULL;
}

void *pong( void *arg ) {
  for ( int i = 0; i < ITERATIONS; i++ ) {
    while ( nextTurn != 1 )
      ;

    nextTurn = 0;
  }
  return NULL;
}

int main( int argc, char *argv[] ) {

  // Create each of the two threads.

  pthread_t pingThread;

  pthread_t pongThread;

  if ( pthread_create( &pingThread, NULL, ping, NULL ) != 0 )
    fail( "Can't create ping thread!" );

  if ( pthread_create( &pongThread, NULL, pong, NULL ) != 0 )
    fail( "Can't create pong thread!" );

  // Wait for them both to finish.

  pthread_join( pingThread, NULL );
  pthread_join( pongThread, NULL );

  return 0;
}
