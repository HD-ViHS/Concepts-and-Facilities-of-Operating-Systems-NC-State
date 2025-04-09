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

// Print out an error message and exit.
static void fail( char const *message )
{
  fprintf( stderr, "%s\n", message );
  exit( 1 );
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
 * @author Siobhan McCarthy
 */
int main( int argc, char *argv[] )
{

  // Simplify the phrasing of the AccountList struct
  typedef struct AccountList AccountList;

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
  } else {
    fail("error");
  }
    
  // User wants to credit an account
  if(strcmp(argv[1], "credit") == 0) {
    for(int i = 0; i < accounts->size; i++) {
      if(strcmp(argv[2], accounts->names[i]) == 0) { // Search for account in shared memory
	if(accounts->balances[i] + bufferNum > MONETARY_LIMIT) // Fail if it overflows account
	  fail("error");
	accounts->balances[i] += bufferNum; // Store new amount in shared memory
	printf("success\n");
	return 0;
      }
    }
    fail("error");
  } else if (strcmp(argv[1], "debit") == 0) { // User wants to debit an account
    for(int i = 0; i < accounts->size; i++) {
      if(strcmp(argv[2], accounts->names[i]) == 0) { // Search for account in shared memory
	if(accounts->balances[i] - bufferNum < 0) // Fail if it underflows account
	  fail("error");
	accounts->balances[i] -= bufferNum; // Store new amount in shared memory
	printf("success\n");
	return 0;
      }
    }
    fail("error");
  } else if (strcmp(argv[1], "query") == 0) { // User wants to query an account
    for(int i = 0; i < accounts->size; i++) {
      if(strcmp(argv[2], accounts->names[i]) == 0) { // Search for account in shared memory
	double doubleToPrint = (double)accounts->balances[i]; // Retrieve correct amount
	doubleToPrint = doubleToPrint / 100; // Change to dollar amount
	if(printf("%.2f\n", doubleToPrint) < 1) { // Print to user
	  fail("error printing query amount");
	}
        return 0;
      }
    }
    fail("error");
  } else {
    fail("error");
  }

  return 0;
}
