#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include "common.h"
#include <sys/stat.h>
#include <semaphore.h>

sem_t *semaphore;

// Simplify the phrasing of the AccountList struct
typedef struct AccountList AccountList;

// Print out an error message and exit.
static void fail( char const *message )
{
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Add the given number of cents to the account with the given name.
// The value of delta should be non-negative
bool credit(AccountList *alist, char const *name, int delta)
{
  #ifndef UNSAFE
    sem_wait(semaphore);
  #endif
  for(int i = 0; i < alist->size; i++) {
    if(strcmp(name, alist->names[i]) == 0) { // Search for account in shared memory
      if(alist->balances[i] + delta > MONETARY_LIMIT) { // Fail if it overflows account
        #ifndef UNSAFE
	  sem_post(semaphore);
        #endif
	return false;
      }
      alist->balances[i] += delta; // Store new amount in shared memory
      #ifndef UNSAFE
        sem_post(semaphore);
      #endif
      return true;
    }
  }
  #ifndef UNSAFE
    sem_post(semaphore);
  #endif
  return false;
}

// Deduct the given number of cents form teh account with the given name.
// The value of delta should be non-negative
bool debit(AccountList *alist, char const *name, int delta )
{
  #ifndef UNSAFE
    sem_wait(semaphore);
  #endif
  for(int i = 0; i < alist->size; i++) {
    if(strcmp(name, alist->names[i]) == 0) { // Search for account in shared memory
      if(alist->balances[i] - delta < 0) // Fail if it underflows account
        #ifndef UNSAFE
	  sem_post(semaphore);
	#endif
	return false;
      alist->balances[i] -= delta; // Store new amount in shared memory
      #ifndef UNSAFE
        sem_post(semaphore);
      #endif
      return true;
    }
  }
  #ifndef UNSAFE
    sem_post(semaphore);
  #endif
  return false;
}

// Return the current balance in teh account with a given name, or
// -1 if the account doesn't exist
int query(AccountList *alist, char const *name )
{
  #ifndef UNSAFE
    sem_wait(semaphore);
  #endif
  for(int i = 0; i < alist->size; i++) {
    if(strcmp(name, alist->names[i]) == 0) { // Search for account in shared memory
      double doubleToPrint = (double)alist->balances[i]; // Retrieve correct amount
      #ifndef UNSAFE
        sem_post(semaphore);
      #endif
      return doubleToPrint / 100; // Change to dollar amount
    }
  }
  #ifndef UNSAFE
    sem_post(semaphore);
  #endif
  return -1;
}

// Test interface, for quickly adding to an account balance and then
// subtracting from the balance
void test(AccountList *alist, char const *name, int n)
{
  for ( int i = 0; i < n; i++)
    credit(alist, name, 1);

  for ( int i = 0; i < n; i++ )
    debit( alist, name, 1 );
}

/**
 * @brief The account function edits the shared memory that hols the user account
 *        names and user balances.
 * 
 * @param argc number of command line arguments
 * @param argv Array of char pointers to command line arguments
 * @return int Success status
 * 
 * @file account.c
 * @author Aidan McCarthy amccart4
 */
int main( int argc, char *argv[] )
{

  // Unique key identifier for shared memory
  key_t shmkey = ftok("/afs/unity.ncsu.edu/users/a/amccart4", 1);

  // Unique ID for shared memory
  int shmId = shmget(shmkey, sizeof(AccountList), 0666);
  if(shmId == -1)
    fail( "Can't create shared memory");
  
  // Allocate an AccountList in shared memory
  AccountList *accounts = (AccountList *)shmat(shmId, NULL, 0);
  if(accounts == (AccountList *)-1 )
    fail("Can't map shared memory into address space" );

  semaphore = sem_open(SEM_NAME, 0);

  // Number for holding the provided double as an int of cents
  int bufferNum = 0;

  // Format checking
  if(argc == 4 &&
     (strcmp(argv[1], "credit") == 0 ||
      strcmp(argv[1], "debit") == 0))  {

    char **test = &(argv[3]); // Char** for error checking after performing strtod()

    // Get the string input as a double
    double amountDouble = strtod(argv[3], test);
    if(amountDouble == 0.0 && **test != '\0')
      fail("error");

    // This simple algorithm comes with credit to
    // https://www.geeksforgeeks.org/rounding-floating-point-number-two-decimal-places-c-c/
    bufferNum = (int)(amountDouble * 100 + .5);

    // Error checking
    if(bufferNum > MONETARY_LIMIT || bufferNum < 0) {
      fail("error");
    }
    
  } else if (argc == 3 && strcmp(argv[1], "query") == 0) {
    // An empty case but a valid one (not bad format)
  } else if (argc == 4 && strcmp(argv[1], "test") == 0) {
    if(sscanf(argv[3], "%d", &bufferNum) < 1)
      fail("error");
  } else {
    fail("error");
  }
    
  // User wants to credit an account
  if(strcmp(argv[1], "credit") == 0) {
    if(credit(accounts, argv[2], bufferNum)) {
      printf("success\n");
    } else {
      fail("error");
    }
  } else if (strcmp(argv[1], "debit") == 0) { // User wants to debit an account
    if(debit(accounts, argv[2], bufferNum)) {
      printf("success\n");
    } else {
      fail("error");
    }
  } else if (strcmp(argv[1], "query") == 0) { // User wants to query an account
    int numToPrint = query(accounts, argv[2]);
    if (numToPrint == -1) {
      fail("error");
    } else if(printf("%.2d\n", numToPrint) != 1) {
      fail("error");
    }
  } else if (strcmp(argv[1], "test") == 0) {
    test(accounts, argv[2], bufferNum);
  } else {
    fail("error");
  }

  return 0;
}
