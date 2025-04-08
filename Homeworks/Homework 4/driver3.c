// Driver program that's simple enough that we can tell what should
// happen.  It has four threads trying to enter the same intersection.  They
// will have to start taking turns.

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

void *hectorThread( void *arg ) {
  const char *name = "hector";
  
  sleep( 1 );   // Wait 1 second before entering.
  
  enterSE( name );
  
  sleep( 4 );   // Then, wait for three seconds before leaving.
  
  leaveSE( name );
  
  return NULL;
}

void *bartThread( void *arg ) {
  const char *name = "bart";
  
  sleep( 2 );   // Wait 2 seconds before trying to enter.
  
  enterSW( name );
  
  sleep( 1 );   // Leave after a second
  
  leaveSW( name );
  
  return NULL;
}

void *jeaneThread( void *arg ) {
  const char *name = "jeane";
  
  sleep( 3 );   // Wait 3 seconds before entering.
  
  enterSE( name );
  
  sleep( 1 );   // Leave after a second
  
  leaveSE( name );
  
  return NULL;
}

void *krystynaThread( void *arg ) {
  const char *name = "krystyna";
  
  sleep( 4 );   // Wait 4 seconds before trying to enter.
  
  enterSW( name );
  
  sleep( 1 );   // Leave after a second
  
  leaveSW( name );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor();

  // Make a few threads
  pthread_t thread[ 4 ];
  if ( pthread_create( thread + 0, NULL, hectorThread, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, bartThread, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, jeaneThread, NULL ) != 0 ||
       pthread_create( thread + 3, NULL, krystynaThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
