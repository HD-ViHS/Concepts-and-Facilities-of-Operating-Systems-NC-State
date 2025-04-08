// Multi-threaded program to simulate a lot of cars wanting to drive
// through the intersection.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for usleep
#include <string.h>    // for strcmp

#include "diamond.h"

/** Bound on how much time threads will wait in the intersecton. */
#define INTERSECTION_TIME 1000

/** Bound on how much time threads will wait before trying to re-enter the intersection. */
#define RESET_TIME 10000

/** Bound on how long it will threads to drive between the east and west
    sides of the intersection. */
#define TRANSIT_TIME 10000

// True as long as the simulation is running.  This is a way to
// tell all the threads when it's time to exit.
int running = 1;

// General purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

/** This record helps to keep up with each thread, including its name,
 * the start routine it will run and the number of times it goes
 * through an intersection. */
typedef struct {
  char name[ 21 ];
  void *(*routine)( void * );
  int count;
} ThreadRec;

/** Funciton for a thread that drives all the way through, west-to-east */
void *westToEast( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter from the west.
    usleep( rand() % RESET_TIME );
    enterNE( tr->name );

    // Take a moment to drive through the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveNE( tr->name );

    // Take a moment to drive to the other side.
    usleep( rand() % TRANSIT_TIME );
    enterSE( tr->name );

    // Take a moment to drive through the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveSE( tr->name );
    
    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

/** Funciton for a thread that drives all the way through, east-to-west */
void *eastToWest( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter from the east
    usleep( rand() % RESET_TIME );
    enterSW( tr->name );

    // Take a moment to drive through the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveSW( tr->name );

    // Take a moment to drive to the other side.
    usleep( rand() % TRANSIT_TIME );
    enterNW( tr->name );

    // Take a moment to drive through the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveNW( tr->name );
    
    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

/** Funciton for a thread that drives north-east through the west side
    of the intersection over and over. */
void *northEast( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter the intersection.
    usleep( rand() % RESET_TIME );
    enterNE( tr->name );

    // Take a moment to drive across the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveNE( tr->name );
    
    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

/** Funciton for a thread that drives north-west through the west side
    of the intersection over and over. */
void *northWest( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter the intersection.
    usleep( rand() % RESET_TIME );
    enterNW( tr->name );

    // Take a moment to drive across the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveNW( tr->name );
    
    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

/** Funciton for a thread that drives south-east through the east side
    of the intersection over and over. */
void *southEast( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter the intersection.
    usleep( rand() % RESET_TIME );
    enterSE( tr->name );

    // Take a moment to drive across the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveSE( tr->name );
    
    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

/** Funciton for a thread that drives soth-west through the east side
    of the intersection over and over. */
void *southWest( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter the intersection.
    usleep( rand() % RESET_TIME );
    enterSW( tr->name );

    // Take a moment to drive across the intersection.
    usleep( rand() % INTERSECTION_TIME );
    leaveSW( tr->name );

    // Count one time through.
    tr->count += 1;
  }

  return NULL;
}

// Struct instance for each thread we'll use.
static ThreadRec threadRec[] = {
  { "Daisy", westToEast, 0 },
  { "Enedina", westToEast, 0 },
  { "Bradly", westToEast, 0 },
  { "Marcella", westToEast, 0 },

  { "Orval", eastToWest, 0 },
  { "Hollie", eastToWest, 0 },
  { "Amos", eastToWest, 0 },
  { "Peter", eastToWest, 0 },
                                
  { "Desmond", northEast, 0 },
  { "Verona", northEast, 0 },
  { "Leonel", northEast, 0 },
  { "Arturo", northEast, 0 },
  
  { "Ellie", northWest, 0 },
  { "Boris", northWest, 0 },
  { "Lorretta", northWest, 0 },
  { "Monroe", northWest, 0 },

  { "Irene", southEast, 0 },
  { "Jeffery", southEast, 0 },
  { "Lucius", southEast, 0 },
  { "Jess", southEast, 0 },
  
  { "Kent", southWest, 0 },
  { "Georgiana", southWest, 0 },
  { "Lily", southWest, 0 },
  { "Rosanna", southWest, 0 },

};

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initMonitor( 10 );

  // Thread id for each of the threads
  int tcount = sizeof( threadRec ) / sizeof( threadRec[ 0 ] );
  pthread_t thread[ tcount ];

  // Make a thread for each car.
  for ( int i = 0; i < tcount; i++ ) {
    if ( pthread_create( thread + i, NULL, threadRec[ i ].routine,
                         threadRec + i ) != 0 )
      fail( "Failed creating a thread\n" );
  }

  // Let them do what they do for a little while.
  sleep( 10 );
  running = 0;

  // Wait for all the threads to finish.
  for ( int i = 0; i < tcount; i++ )
    pthread_join( thread[ i ], NULL );

  
  // Report how many times each thread went through the intersection and report
  // a total.
  int total = 0;
  for ( int i = 0; i < tcount; i++ ) {
    // Report how many times this thread went through the intersection.
    printf( "%s drove through %d times\n", threadRec[ i ].name,
            threadRec[ i ].count );
    total += threadRec[ i ].count;
  }
  printf( "Total: %d\n", total );

  // Free any monitor resources.
  destroyMonitor();


  return 0;
}
