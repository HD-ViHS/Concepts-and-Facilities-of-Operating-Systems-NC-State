#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( EXIT_FAILURE );
}

// Print out a usage message, then exit.
static void usage() {
  printf( "usage: zerosum <workers>\n" );
  printf( "       zerosum <workers> report\n" );
  exit( 1 );
}

// Semaphore for keeping critical sections related to the
// countedIndices variable
sem_t countedLock;

// Semaphore for blocking when no data available
sem_t readData;

// Semaphore for protecting overall total
sem_t totalLock;

// Semaphore for protecting vList
sem_t inputLock;

// True if we're supposed to report what we find.
bool report = false;

// Total number of subsequences found.
int total = 0;

// Fixed-sized array for holding the sequence.
#define MAX_VALUES 500000
int vList[ MAX_VALUES ];

// Current number of values on the list.
int vCount = 0;

// Keep track of what index to assign a new worker
int countedIndices = 0;

// To keep track of when readList is done reading
bool finished = false;

// Read the list of values
// The int param is to affect the number of sentinel
// values which get printed in the buffer
void readList(int workers) {
  // Keep reading as many values as we can.
  int v;
  while ( scanf( "%d", &v ) == 1 ) {
    // Make sure we have enough room, then store the latest input.
    if ( vCount > MAX_VALUES )
      fail( "Too many input values" );

    vList[ vCount++ ] = v; // Edit the buffer

    sem_post(&readData); // Let workers know there's new data
  }
  if(vCount < MAX_VALUES) {  // Done with input
    for(int i = 0; i <= workers; i++) {
      vList[vCount + i] = INT_MAX;
      sem_post(&readData);
    }
  }
}

/** Returns index for workers to work on */
int getWork() {

  if(vList[countedIndices] == INT_MAX) {
    return -1;
  }
  
  sem_wait(&readData); // Wait for new data
  
  int index = countedIndices++; // Get return value and increment amount worked on
  return index; 
}

/** Start routine for each worker. */
void *workerRoutine( void *arg ) {

  sem_wait(&countedLock);
  int index = getWork(); // Get your index
  sem_post(&countedLock);
  
  while(index != -1) { // While there's work
    // Calculate all possible strings involving that index as the "end"
    int localTotal = 0;
    for(int i = index; i >= 0; i--) {

      localTotal += vList[i];
      
      if(localTotal == 0) {   // We found a sequence
	sem_wait(&totalLock); // Keep any other thread from touching total
	total++;              // Increment the total
	sem_post(&totalLock); // Let other threads touch total
	if(report) {          // Report the value if necessary
	  printf("%d .. %d\n", i, index);
	}
      }
    }
    sem_wait(&countedLock);
    index = getWork();
    sem_post(&countedLock);
  }
  return NULL;
}

int main( int argc, char *argv[] ) {
  int workers = 4;
  
  // Parse command-line arguments.
  if ( argc < 2 || argc > 3 )
    usage();
  
  if ( sscanf( argv[ 1 ], "%d", &workers ) != 1 ||
       workers < 1 )
    usage();

  // If there's a second argument, it better be "report"
  if ( argc == 3 ) {
    if ( strcmp( argv[ 2 ], "report" ) != 0 )
      usage();
    report = true;
  }

  // Initialize Semaphores
  sem_init(&inputLock, 0, 1);
  sem_init(&countedLock, 0, 1);
  sem_init(&totalLock, 0 , 1);
  sem_init(&readData, 0, 0);
  
  // Make each of the workers.
  pthread_t worker[ workers ];
  for ( int i = 0; i < workers; i++ ) {
    pthread_create(&(worker[i]), NULL, &workerRoutine, NULL);
  }

  // Then, start getting work for them to do.
  readList(workers);

  // Wait until all the workers finish.
  for ( int i = 0; i < workers; i++ ) {
    pthread_join(worker[i], NULL);
  }

  // Report the total and release the semaphores.
  printf( "Total: %d\n", total );

  // Destroy Semaphores
  sem_destroy(&inputLock);
  sem_destroy(&countedLock);
  sem_destroy(&totalLock);
  sem_destroy(&readData);
  
  return EXIT_SUCCESS;
}
