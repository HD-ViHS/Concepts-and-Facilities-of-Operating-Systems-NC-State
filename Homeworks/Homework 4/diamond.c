#include "diamond.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>

/** Block access to the monitor to one thread at a time */
pthread_mutex_t lock;

// Signals for cars exiting intersections
// One for the eastern intersection (cars leave going north)
pthread_cond_t northSignal;
// One for the western intersection  (cars leave going south)
pthread_cond_t southSignal;

/** How many cars are in the intersection in each direction */
static int carsInNE;
static int carsInNW;
static int carsInSE;
static int carsInSW;

/** How many cars are waiting in a direction in an intersection */
static int carsWaitingNE;
static int carsWaitingNW;
static int carsWaitingSE;
static int carsWaitingSW;

/** What direction did the last car come from?
  *  Important for turn taking.
  */
static bool lastCarNE;
static bool lastCarNW;
static bool lastCarSE;
static bool lastCarSW;


/** Initialize the monitor. Initialize the mutex, all the
  * condition variables, and the state of the monitor
  */
void initMonitor() {
  // Initialize the mutex
  if (pthread_mutex_init(&lock, NULL) != 0) {
    printf("\nmutexinit has failed\n");
    return;
  }

  // Initialize the signal for leaving in NE direction
  if (pthread_cond_init(&northSignal, NULL) != 0) {
    printf("\nnCondInit has failed\n");
    return;
  }

  // Initialize the signal for leaving in SW direction
  if (pthread_cond_init(&southSignal, NULL) != 0) {
    printf("\nsCondInit has failed\n");
    return;
  }

  /** No cars are driving yet */
  carsInNE = 0;
  carsInNW = 0;
  carsInSE = 0;
  carsInSW = 0;

  /** No cars are waiting yet */
  carsWaitingNE = 0;
  carsWaitingNW = 0;
  carsWaitingSE = 0;
  carsWaitingSW = 0;

  /** No cars have entered, so none have left */
  lastCarNE = false;
  lastCarNW = false;
  lastCarSE = false;
  lastCarSW = false;
  
}

/** Destroys all the initialized monitor state (mutex and
  * condition variables).
  */
void destroyMonitor() {
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&northSignal);
  pthread_cond_destroy(&southSignal);
}

/** Function to enter the the intersection in a NE direction. */
void enterNE( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car is now waiting to enter the intersection
  carsWaitingNE++;

  // If a car is in the intersection going the other direction
  // or if the last car to enter came from the same direction
  // and now we should give the car going the other direction
  // a turn, wait.
  while ( carsInNW > 0 || ((carsWaitingNW > 0) && lastCarNE)) {
    pthread_cond_wait(&northSignal, &lock);
  }

  // The car is no longer waiting to enter the intersection...
  carsWaitingNE--;

  lastCarNE = true;
  lastCarNW = false;
  
  // It has entered the intersection
  carsInNE++;
  
  // Declare its entrance to the world
  printf("Entering NE: %s\n", name);
  
  // Exit the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to enter the the intersection in a NW direction. */
void enterNW( char const *name ) {
  // Enter the Monitor
  pthread_mutex_lock(&lock);

  // The car is now waiting in the intersection
  carsWaitingNW++;

  // If a car is in the intersection going the other direction
  // or if the last car to enter came from the same direction
  // and now we should give the car going the other direction
  // a turn, wait.
  while ( carsInNE > 0 || ((carsWaitingNE > 0) && lastCarNW)) {
    pthread_cond_wait(&northSignal, &lock);
  }

  // The car is no longer waiting...
  carsWaitingNW--;

  lastCarNW = true;
  lastCarNE = false;

  // It has entered the intersection
  carsInNW++;

  // Declare its entrance to the world
  printf("Entering NW: %s\n", name);

  // Leave the Monitor
  pthread_mutex_unlock(&lock);
}

/** Function to enter the the intersection in a NE direction. */
void enterSE( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car is waiting to enter the intersection
  carsWaitingSE++;

  // If a car is in the intersection going the other direction
  // or if the last car to enter came from the same direction
  // and now we should give the car going the other direction
  // a turn, wait.
  while ( carsInSW > 0 || ((carsWaitingSW > 0) && lastCarSE)) {
    pthread_cond_wait(&southSignal, &lock);
  }

  // The car is no longer waiting in the intersection...
  carsWaitingSE--;

  lastCarSE = true;
  lastCarSW = false;

  // It has entered the intersection
  carsInSE++;

  // Declare its entrance to the world
  printf("Entering SE: %s\n", name);

  // Exit the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to enter the the intersection in a NE direction. */
void enterSW( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car is waiting at the intersection
  carsWaitingSW++;

  // If a car is in the intersection going the other direction
  // or if the last car to enter came from the same direction
  // and now we should give the car going the other direction
  // a turn, wait.
  while ( carsInSE > 0 || ((carsWaitingSE > 0) && lastCarSW)) {
    pthread_cond_wait(&southSignal, &lock);
  }

  // The car is no longer waiting at the intersection...
  carsWaitingSW--;

  // It is in the intersection
  carsInSW++;

  lastCarSW = true;
  lastCarSE = false;

  // Declare its entrance to the world
  printf("Entering SW: %s\n", name);

  // Exit the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to exit the the intersection in a NE direction. */
void leaveNE( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car is no longer in the intersection
  carsInNE--;

  // Declare its leaving to the world
  printf("Leaving NE: %s\n", name);

  // Signal to the other cars that it has left the intersection
  pthread_cond_broadcast( &northSignal );

  // Exit the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to exit the the intersection in a NW direction. */
void leaveNW( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car has left the intersection
  carsInNW--;

  // Declare exit to the world
  printf("Leaving NW: %s\n", name);

  // Signal to other cars in the intersection that it has left
  pthread_cond_broadcast( &northSignal );

  // Exit the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to exit the the intersection in a SE direction. */
void leaveSE( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car has left the intersection
  carsInSE--;

  // Declare leaving to the world
  printf("Leaving SE: %s\n", name);

  // Signal to other cars it has left the intersection
  pthread_cond_broadcast( &southSignal );

  // Leave the monitor
  pthread_mutex_unlock(&lock);
}

/** Function to exit the the intersection in a SW direction. */
void leaveSW( char const *name ) {
  // Enter the monitor
  pthread_mutex_lock(&lock);

  // The car is no longer in the intersection
  carsInSW--;

  // Declare leaving to the world
  printf("Leaving SW: %s\n", name);

  // Signal to other cars that it left the intersection
  pthread_cond_broadcast( &southSignal );

  // Leave the monitor
  pthread_mutex_unlock(&lock);
}
