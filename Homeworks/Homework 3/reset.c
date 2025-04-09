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
#include <semaphore.h>
#include <sys/stat.h>

sem_t *semaphore;

// Print out an error message and exit.
static void fail( char const *message )
{
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

// Print out a usage message and exit.
static void usage()
{
  fprintf( stderr, "usage: reset (<account-name> <balance>)+\n" );
  exit( EXIT_FAILURE );
}

/**
 * @brief The reset function creates the shared memory for holding the user account
 *        names and user balances.
 * 
 * @param argc number of command line arguments
 * @param argv Array of char pointers to command line arguments
 * @return int Success status
 * 
 * @file reset.c
 * @author Siobhan McCarthy
 */
int main( int argc, char *argv[] )
{

  // Check for valid line pre-parsing (should have
  // an odd number of arguments (counting "./server"),
  // should represent between 1 and 10 accounts.
  if ( (argc % 2) == 0 ||
       argc < ( (2 * ALIST_MIN) + 1 ) ||
       argc > ( (2 * ALIST_MAX) + 1 )) {
    usage();
  }

  int numOfAccounts = (argc - 1)/2;

  char nameBuffer[numOfAccounts][ANAME_MAX];
  int numBuffer[numOfAccounts];
  
  for(int i = 1; i < argc; i++) { // Begin with second argument
    if(i % 2 != 0) { // account name
      if(strlen(argv[i]) > ANAME_MAX )
	      usage();

      for(int j = 0; j < numOfAccounts; j++) {
	if(strcmp(nameBuffer[j], argv[i]) == 0) // No duplicate account names
	  usage();
      }

      // Store name in struct (i/2 because twice as many args as accounts)
      strcpy(nameBuffer[(i/2)], argv[i]); 
      
    } else { // account amount
      int bufferNum = 0; // Digit to store amount as num
      char **test = &(argv[i]); // Char** for error checking after performing strtod()

      double amountDouble = strtod(argv[i], test);
      if(amountDouble == 0.0 && **test != '\0')
	usage();

      // This simple algorithm comes with credit to
      // https://www.geeksforgeeks.org/rounding-floating-point-number-two-decimal-places-c-c/
      bufferNum = (int)(amountDouble * 100 + .5);

      if(bufferNum > MONETARY_LIMIT)
	usage();

      numBuffer[(i/2) - 1] = bufferNum;
    }

  }

    // If program gets to this point, then all data is valid and can be
    // placed into shared memory

    typedef struct AccountList AccountList;

    // Unique key identifier for shared memory
    key_t shmkey = ftok("/afs/unity.ncsu.edu/users/a/amccart4", 1);

    // Create new shared memory for the struct
    int shmId = shmget(shmkey, sizeof(AccountList), 0666 | IPC_CREAT);
    
    if(shmId == -1)
      fail("Failure creating shared memory");
  
    AccountList *accounts = (AccountList *)shmat(shmId, NULL, 0);
    
    if(accounts == (AccountList *)-1)
      fail("Falure attatching shared memory"); 

    // Give size of list to struct
    accounts->size = numOfAccounts;
    
    for(int i = 0; i < numOfAccounts; i++) {
      accounts->balances[i] = numBuffer[i]; // Update struct's balances
      strcpy(accounts->names[i], nameBuffer[i]); // Update structs names
    }
  return 0;
}
