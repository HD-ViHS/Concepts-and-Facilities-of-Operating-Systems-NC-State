#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
/** The size of the failure message in bytes */
#define ERR_MESSAGE_SIZE 57
/** The size of the blocks to read input from in bytes */
#define BLOCK_SIZE 64
/** The ASCII value of '0' */
#define ASCII_ZERO 48

/**
 * A fail function that does not close any files for sake of simplicity in the main program.
 */
void failBeforeCreation()
{
    char errMessage[ERR_MESSAGE_SIZE] = "usage: exclude <input-file> <output-file> <line-number>\n"; // Error Message to print to stderr

    write(STDERR_FILENO, errMessage, ERR_MESSAGE_SIZE); // Write the Error Message to stderr (STDERR_FILENO is the constant for the file number of stderr)
    _exit(1);
}

/**
 * A fail function that closes the files pased as parameters for sake of simplicity in the main program.
 * 
 * @param inputHandle the handle of the input file to close
 * @param outputHandle the handle of the output file to close
 */
void failAfterCreation(int inputHandle, int outputHandle)
{
    char errMessage[ERR_MESSAGE_SIZE] = "usage: exclude <input-file> <output-file> <line-number>\n"; // Error Message to print to stderr

    close(inputHandle);
    close(outputHandle);

    write(STDERR_FILENO, errMessage, ERR_MESSAGE_SIZE); // Write the Error Message to stderr (STDERR_FILENO is the constant for the file number of stderr)
    _exit(1);
}

/**
 * The exclude.c program takes an input file, an output file, and a number
 * and prints the text from the input file to the output file with the exception
 * of (exclusion of) the line numbered by the number provided.
 * 
 * @file exclude.c
 * @param argc the number of arguments (should always be 4 in this case)
 * @param argv the array of char pointers that is the command line arguments
 * @return int the exit status
 */
int main(int argc, char *argv[])
{

    if (argc != 4) // Checks for correct number of arguments
    {
        failBeforeCreation();
    }

    int line = 0;
    int length = 0;
    while (argv[3][length] != '\0')
    {
        length++;
    }

    int multiplier = 1;
    for(int i = length - 1; i >= 0; i--)
    {
        if(argv[3][i] > (ASCII_ZERO + 9) || argv[3][i] < ASCII_ZERO) // Tests that ASCII value is number 0-9
        {
            failBeforeCreation();
        }

        line += ((argv[3][i] - ASCII_ZERO) * multiplier); // Accumulate digital value of line argument
        multiplier *= 10; // Multiply digit space
    } // At this point, the int 'line' should correctly hold the integer value of the line to exclude
    line -= 1; // However, since counting *actually* starts at 0, we need to decrement the line value to remove the correct line.

    int currentLine = 0; // Counter for reading through input and finding line numbers

    char readBuffer[BLOCK_SIZE]; // Buffer to read text from
    char writeBuffer[BLOCK_SIZE]; // Buffer to write text from

    int inputHandle = open(argv[1], O_RDONLY);

    int bytesRead = read(inputHandle, readBuffer, BLOCK_SIZE);
    int outputHandle = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 00700); // Open output file for write only, creating a new file if one doesn't already exist, with user-only read and write permissions

    if (inputHandle == -1 || outputHandle == -1 || bytesRead == -1) // Checks for failure of read() and open() functions
    {
        failAfterCreation(inputHandle, outputHandle);
    }

    while (bytesRead > 0)
    {

        int readIndex = 0;
        int writeIndex = 0;
        while(readIndex < bytesRead) // Ensure that only the amount of chars read are interacted with
        {
            if (currentLine == line) // Check for line to exclude
            {
	            while (readBuffer[readIndex] != '\n' && readIndex < bytesRead) // Skip everything up to next line or until buffer limit reached
                {
                    readIndex++; // Skipping indeces
                }
		        if (readIndex >= bytesRead)
		            break;
                else
                {
                    readIndex++; // Advance past '\n' char of excluded line
                    currentLine++; // Increment the line count
                }
            }
	        if(readIndex >= bytesRead)
	            break;

	        if (readBuffer[readIndex] == '\n') // Check for EOL
	            currentLine++; // Increment line count

            writeBuffer[writeIndex] = readBuffer[readIndex]; // Write from input buffer to output buffer
            writeIndex++; // Increment output buffer index
            readIndex++; // Increment input buffer index
        }
	    // At this point, the write buffer should contain all characters to write from the read buffer, whether or not that excludes the appropriate line
        
        write(outputHandle, writeBuffer, writeIndex); // Write to output
        bytesRead = read(inputHandle, readBuffer, BLOCK_SIZE); // Read from input

	    if(bytesRead == -1) // Checks for failure of read() function
        {
	        failAfterCreation(inputHandle, outputHandle);
        }

    }

    close(inputHandle);
    close(outputHandle);

    return 0; // Successful exit
}
