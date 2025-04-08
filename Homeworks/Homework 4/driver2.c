// Driver program that's simple enough that we can tell what should
// happen.  It has two threads trying to go through the east intersection
// in the same direction.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep

#include "diamond.h"

// General-purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

void *susanThread( void *arg ) {
  const char *name = "susan";
  
  sleep( 1 );   // Wait 1 second before entering.
  
  enterSE( name );
  
  sleep( 2 );   // Then, wait for two seconds before leaving.
  
  leaveSE( name );
  
  return NULL;
}

void *orvilleThread( void *arg ) {
  const char *name = "orville";
  
  sleep( 2 );   // Wait 2 seconds before trying to enter.
  
  enterSE( name );
  
  sleep( 2 );   // Try to leave after two more seconds.
  
  leaveSE( name );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor();

  // Make a few threads
  pthread_t thread[ 2 ];
  if ( pthread_create( thread + 0, NULL, susanThread, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, orvilleThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
