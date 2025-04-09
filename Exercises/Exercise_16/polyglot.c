#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

/**
 * Takes the command line object files and runs their getMessage()
 * function. It only works if the object files have a getMessage()
 * function that returns a char* and contains no parameters.
 *
 * @file polyglot.c
 * @author Siobhan McCarthy
 */
int main( int argc, char *argv[] )
{

  for(int i = 1; i < argc; i++) {

    char* (*functionDummy)();
    void *handle;

    // When a shared object is called, it needs this at the
    // of its pathname.

    char fileBeginning[] = "./";

    // Create the relative pathname
    strcat(fileBeginning, argv[i]);

    // Get the handle
    handle = dlopen(fileBeginning, RTLD_LAZY);
    
    // Return an error message on failure to obtain handle
    if (!handle) {
      fprintf(stderr, "Error: %s\n", dlerror());
      return EXIT_FAILURE;
    }

    // Get the getMessage() function
    functionDummy = dlsym(handle, "getMessage");

    // Run the getMessage() function
    char* message = functionDummy();
    
    // Print the message
    printf("%s\n", message);

    // Free the message
    free(message);

    // Close the handle
    dlclose(handle);
  }

  return EXIT_SUCCESS;
}
