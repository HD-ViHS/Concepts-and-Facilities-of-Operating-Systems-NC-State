#include "common.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

/**
 * Processes user input for validity (purely in terms of syntax),
 * sends the command to the server, recieves the response, returns
 * the status of the response, then exits.
 *
 * @file client.c
 * @author Aidan McCarthy amccart4
 */
int main ( int argc, char *argv[] ) {


  // Prepare structure indicating maximum queue and message sizes.
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = MESSAGE_LIMIT;

  char outputBuffer[MESSAGE_LIMIT];
  char inputBuffer[MESSAGE_LIMIT];

  // Create server and client message queues
  mqd_t serverQueue = mq_open( SERVER_QUEUE, O_WRONLY | O_CREAT, 0600, &attr );
  mqd_t clientQueue = mq_open( CLIENT_QUEUE, O_RDONLY | O_CREAT, 0600, &attr );

  // If the command is valid from a purely syntax perspective
  if ((((strcmp(argv[1], "credit") == 0) || (strcmp(argv[1], "debit") == 0)) && (argc == 4)) ||
      (strcmp(argv[1], "query") == 0 && argc == 3)) {
    int outputIndex = 0;
    for(int i = 1; i < argc; i++) { // For each argument
      for(int j = 0; j < strlen(argv[i]); j++) { // For each char in the arg
	outputBuffer[outputIndex] = argv[i][j];
	outputIndex++;
	if(outputIndex >= MESSAGE_LIMIT) { // Over message size limit
	  printf("error\n");
	  mq_close( clientQueue );
	  mq_close( serverQueue );
	  exit(EXIT_FAILURE);
	}
      }
      outputBuffer[outputIndex] = ' '; // Put spaces in betweeen args
      outputIndex++;
      if(outputIndex > MESSAGE_LIMIT) { // Over message size limit
	printf("error\n");
	mq_close( clientQueue );
	mq_close( serverQueue );
	exit(EXIT_FAILURE);
      }
    }
    // Null Terminate the input string
    // (index currently points one space
    // too far)
    outputBuffer[outputIndex - 1] = '\0';
  } else {
    printf("error\n");
    exit(EXIT_FAILURE);
  }

  // Send the command to the server as a space-separated,
  // null terminated string.
  mq_send(serverQueue, outputBuffer, sizeof(outputBuffer), 0);

  // Get back the response from the server
  mq_receive(clientQueue, inputBuffer, sizeof(inputBuffer), NULL);

  // On "credit" or "debit" inputs, no reports are necessary besides
  // the status of the operation
  if(strcmp(argv[1], "credit") == 0 || strcmp(argv[1], "debit") == 0) {
    if(strcmp(inputBuffer, "error") == 0) {
      printf("error\n");
      mq_close( clientQueue );
      mq_close( serverQueue );
      exit(EXIT_FAILURE);
    } else if (strcmp(inputBuffer, "success") == 0) {
      printf("success\n");
    } else {
      printf("Error recieving message queue\n");
      mq_close( clientQueue );
      mq_close( serverQueue );
      exit(EXIT_FAILURE);
    }
    // On "query" inputs, the actual data is returned
    // from the server
  } else if(strcmp(argv[1], "query") == 0) {
    char *reference = &(inputBuffer[0]);
    char **test = &(reference);
    double doubleToPrint = strtod(inputBuffer, test);
    if(doubleToPrint == 0.0 && **test != '\0') {
      printf("Error parsing double from query\n");
    }
    if(printf("%.2f\n", doubleToPrint) < 1) {
      printf("Error recieving message queue");
      mq_close( clientQueue );
      mq_close( serverQueue );
      exit(EXIT_FAILURE);
    }
  }
  mq_close( clientQueue );
  mq_close( serverQueue );
  return 0;
}
