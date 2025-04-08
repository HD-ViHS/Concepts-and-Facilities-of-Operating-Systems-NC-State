#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <regex.h>
#include <semaphore.h>
#include <limits.h>

/** Port number used by my server */
#define PORT_NUMBER "28123"

/** Maximum length of a user name. */
#define NAME_LEN 10

/** Longest command is 6 letters ("submit" or "report")
 *  plus the 24 character word limit plus the space in
 *  between the commands = 31. */
#define MAX_COMMAND_LENGTH 31

sem_t semaphore;

typedef struct {
  char **words;
  char **users;
  int *values;
  int length;
  int capacity;
} WordList;

// Print out an error message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

void initWordList(WordList *list, size_t initialSize) {
  list->words = (char **)malloc(initialSize * sizeof(char*));
  list->users = (char **)malloc(initialSize * sizeof(char *));
  list->values = (int *)malloc(initialSize * sizeof(int));
  list->length = 0;
  list->capacity = initialSize;
}

int calculateValue(char *word) {
  int total = 0;

  // I know this is atrocious code, i might fix it with regex later or something
  for(int i = 0; i < strlen(word)) {
    
    if(isalpha(word[i]) == 0) {
      return -1;
    }
    
    if(word[i] == 'A' || word[i] == 'a' ||
       word[i] == 'E' || word[i] == 'e' ||
       word[i] == 'I' || word[i] == 'i' ||
       word[i] == 'O' || word[i] == 'o' ||
       word[i] == 'U' || word[i] == 'u' ||
       word[i] == 'L' || word[i] == 'l' ||
       word[i] == 'N' || word[i] == 'n' ||
       word[i] == 'S' || word[i] == 's' ||
       word[i] == 'T' || word[i] == 't' ||
       word[i] == 'R' || word[i] == 'r') {
      
      
      total += 1;
      
    } else if (word[i] == 'D' || word[i] == 'd' ||
	       word[i] == 'G' || word[i] == 'g') {
      
      total += 2;
      
    } else if (word[i] == 'B' || word[i] == 'b' ||
	       word[i] == 'C' || word[i] == 'c' ||
	       word[i] == 'M' || word[i] == 'm' ||
	       word[i] == 'P' || word[i] == 'p') {
      
      total += 3;
      
    } else if (word[i] == 'F' || word[i] == 'f' ||
	       word[i] == 'H' || word[i] == 'h' ||
	       word[i] == 'V' || word[i] == 'v' ||
	       word[i] == 'W' || word[i] == 'w' ||
	       word[i] == 'Y' || word[i] == 'y') {
      
      total += 4;
      
    } else if (word[i] == 'K' || word[i] == 'k') {
      
      total += 5;
      
    } else if (word[i] == 'J' || word[i] == 'j' ||
	       word[i] == 'X' || word[i] == 'x') {
      
      total += 8;
      
    } else if (word[i] == 'Q' || word[i] == 'q' ||
	       word[i] == 'Z' || word[i] == 'z') {
      
      total += 10;
    }
  }
  return total;
}

void insertNewWord(WordList *list, char *word, char *userName) {
  sem_wait(&semaphore);
  if(list->length == list->capacity) {
    list->size *= 2;
    list->array = realloc(list->array, list->size * sizeof(char*));
  } 
  list->array[list->length] = word;
  list->users[list->length] = userName;
  list->values[list->length++] = calculateValue(word);
  sem_post(&semaphore);
}

void insertReplacementWord(WordList *list, char *word) {
  sem_wait(&semaphore);
  for(int i = 0; i < list->length; i++) {
    if(strcmp(list->words[i], word) == 0) {
      list->words[i] = word;
      list->values[i] = calculateValue(word);
      sem_post(&semaphore);
      return;
    }
  }
  sem_post(&semaphore);
  fail("Bro, how the h*ck did you get here?");
}

void freeWordList(WordList *list) {
  for(int i = 0; i < length; i++) {
    free(list->array[i]);
  }
  free(list->array);
  free(list->values);
  list->array = NULL;
  list->values = NULL;
  list->length = list->capacity = 0;
}

/** handle a client connection, close it when we're done. */
void *handleClient( int sock, WordList *list ) {
  // Here's a nice trick, wrap a C standard IO FILE around the
  // socket, so we can communicate the same way we would read/write
  // a file.
  FILE *fp = fdopen( sock, "a+" );
  
  // Prompt the user for a name.
  fprintf( fp, "username> " );

  char user[ NAME_LEN + 2];
  if ( fgets( user, ( NAME_LEN + 1 ), fp ) == NULL) { // Max name length plus null char
    fprintf( fp, "Invalid username\n" );
    fclose( fp );
    return NULL;
  }
  
  bool validUsername = false; // Keeping track of the validity of the username
  for(int i = 0; i <= NAME_LEN; i++) {
    if(user[i] < 'A' || user[i] > 'Z' || user[i] != '\0') // All chars should be capital letters or null
      break; // validStr will still be false
    
    if(user[i] == '\0' && i > 0) { // If the string ends with a user name of length 1-10
      validUsername = true;
      break;
    }
  }

  if(!validUsername) {
    fprintf( fp, "Invalid username\n" );
    fclose( fp );
    return NULL;
  }
  
  // Prompt the user for a command.
  fprintf( fp, "cmd> " );

  // Temporary values for parsing commands.
  char cmd[ MAX_COMMAND_LENGTH + 2 ]; // Max command length and null char (and one extra for my personal safety superstition)

  // Has this thread submitted 
  bool submitted = false;
  
  while ( strcmp( cmd, "quit" ) != 0 ) {

    if ( fgets( cmd, ( MAX_COMMAND_LENGTH + 1 ), fp ) == NULL) { // Max command length plus null char
      fprintf( fp, "Invalid command\n" );
      fprintf( fp, "cmd> ");
      continue;
    }
  
    bool validCommand = false; // Keeping track of the validity of the username
    for(int i = 0; i <= MAX_COMMAND_LENGTH; i++) {
      if(user[i] == '\0' && i > 3) { // The shortest command (quit) still requires 4 letters
	validCommand = true;
	break;
      }
    }
    
    if(!validCommand) {
      fprintf( fp, "Invalid command\n" );
      fprintf( fp, "cmd> ");
      continue;
    }

    char *commandList[2];
    commandList[0] = cmd[0];
    for(int i = 0; i < MAX_COMMAND_LENGTH; i++) {
      if(cmd[i] == ' ' && cmd[i+1] != '\0') {
	cmd[i] = '\0';
	commandList[1] = cmd[i+1];
	break;
      }
    }

    if(strcmp("query", commandList[0]) == 0) {
      if( commandList[1] != NULL ) {
	int total = calculateValue(commandList[1]);
	printf("%d\n", &total);
      } else { // User gave query command with no follow-up word
	fprintf( fp, "Invalid command\n" );
	fprintf( fp, "cmd> ");
        continue;
      }
    } else if(strcmp("submit", commandList[0]) == 0) {
      for(int i = 0; i < strlen(commandList[1])) {
	if(isalpha(commandList[1][i]) == 0) {
	  fprintf( fp, "Invalid command\n" );
	  fprintf( fp, "cmd> ");
	  break;
	}
      }
      if(submitted) {
	insertReplaceMentWord(&list, commandList[1]);
      } else {
	insertNewWord(&list, commandList[1], user);
	submitted = true;
      }
      
    } else if(strcmp("report", commandList[0]) == 0) {
      int min = INT_MAX;

      sem_wait(&semaphore);

      int n = list->length;

      char *sortedList[n];
      int sortedValues[n];
      char *sortedNames[n];

      memcpy(&sortedList, &(list->words), n * sizeof(char *));
      memcpy(&sortedValues, &(list->values), n * sizeof(int));
      memcpy(&sortedNames, &(list->users), n * sizeof(char *));
      sem_post(&semaphore);

      int tempInt = 0;
      char *tempWord;
      char *tempName;

      // Selection sort to provide words in order from least to greatest value
      for(int i = 0; i < n; ++i) {
	for (j = i + 1; j < n; ++j) {
	  if(sortedValues[i] > sortedValues[j]) {
	    tempInt = sortedValues[i];	    
	    sortedValues[i] = sortedValues[j];
	    sortedValues[j] = tempInt;

	    tempWord = sortedList[i];
	    sortedWords[i] = sortedWords[j];
	    sortedWords[j] = tempWord;

	    tempName = sortedNames[i];
	    sortedNames[i] = sortedNames[j];
	    sortedNames[j] = tempName;
	  }
	}
      }
      for(int i = 0; i < n; i++) {
	printf("%10s %24s %3d", sortedNames[i], sortedWords[i], &(sortedValues[i]));
      }
      
    } else if(strcmp("quit", commandList[0]) == 0) {
	// Placeholder for a valid case that's already handled by the while loop
    } else {
      fprintf( fp, "Invalid command\n" );
      fprintf( fp, "cmd> ");
      continue;
    }

  }

  // Close the connection with this client.
  fclose( fp );
  return NULL;
}

int main() {
  // Prepare a description of server address criteria.
  struct addrinfo addrCriteria;
  memset(&addrCriteria, 0, sizeof(addrCriteria));
  addrCriteria.ai_family = AF_INET;
  addrCriteria.ai_flags = AI_PASSIVE;
  addrCriteria.ai_socktype = SOCK_STREAM;
  addrCriteria.ai_protocol = IPPROTO_TCP;

  // Lookup a list of matching addresses
  struct addrinfo *servAddr;
  if ( getaddrinfo( NULL, PORT_NUMBER, &addrCriteria, &servAddr) )
    fail( "Can't get address info" );

  // Try to just use the first one.
  if ( servAddr == NULL )
    fail( "Can't get address" );

  // Create a TCP socket
  int servSock = socket( servAddr->ai_family, servAddr->ai_socktype,
                         servAddr->ai_protocol);
  if ( servSock < 0 )
    fail( "Can't create socket" );

  // Bind to the local address
  if ( bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0 )
    fail( "Can't bind socket" );
  
  // Tell the socket to listen for incoming connections.
  if ( listen( servSock, 5 ) != 0 )
    fail( "Can't listen on socket" );

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  // Fields for accepting a client connection.
  struct sockaddr_storage clntAddr; // Client address
  socklen_t clntAddrLen = sizeof(clntAddr);

  WordList wordList;
  initWordList(&wordList, 4);

  while ( true  ) {
    // Accept a client connection.
    int sock = accept( servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);

    // Talk to this client.
    handleClient( sock, &wordList );
  }

  freeWordList(&wordList);
  
  // Stop accepting client connections (never reached).
  close( servSock );
  
  return 0;
}
