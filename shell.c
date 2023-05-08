// McGill University
// COMP 310 - ECSE 427
// Assignment 3: Memory Management
// Authors:
// Rambod Azimi - 260911967
// Cindy Irawan - 261132839

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
//#include <unistd.h>
#include "interpreter.h"
#include "shellmemory.h"

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

// Compilation flags
int var_size = VAR_STORE_SIZE;
int frame_size = FRAME_STORE_SIZE;

// Start of everything
int main(int argc, char *argv[]) {

    // Welcoming message to the user
	printf("%s\n", "Shell version 1.2 Created January 2023\n");
    printf("Frame Store Size = %d; Variable Store Size = %d\n", frame_size, var_size);
	remove_backingstore(); // Removing the backingstore directory

	char prompt = '$';  				// Shell prompt
	char userInput[MAX_USER_INPUT];		// User's input stored here
	int errorCode = 0;					// Zero means no error, default

	// Initializing user's input to '\0' character
	for (int i = 0; i < MAX_USER_INPUT; i++)
		userInput[i] = '\0';
	
	// Init the whole shell memory to "none" by calling mem_init function in the shellmemory.c
	mem_init();
	while(1) {							
		//printf("%c ", prompt);
        // Here you should check the unistd library 
        // So that you can find a way to not display $ in the batch mode
		fgets(userInput, MAX_USER_INPUT - 1, stdin); // Get an string input from the user and save it into userInput array

        // One-Liners
        if((strstr(userInput, ";") != NULL)) { // If the userInput contains semicolon, run the one-liner code to process each token separately
            int commandLength = 0; // Holding the number of commands
            // Returns the first token separated by semicolon
            char *token = strtok(userInput, ";");
            char *commandArray[10]; // Saving each command in one index of array to process them individually
            // Keep tokenizing commands separated by semicolon
            while (token != NULL) {
                if(token[0] == ' ') { // If the command starts with space, remove it
                    token++;
                }
                commandArray[commandLength] = token;
                commandLength++;
                token = strtok(NULL, ";");
            }
            if (commandLength > 10) {
                printf("%s\n", "Badcommand: Too many commands in one line");
                continue;
            }

            for(int i = 0 ; i < commandLength ; i++) { // Iterate over each command and execute them individually
               	errorCode = parseInput(commandArray[i]); // Calls parseInput function below
		        if (errorCode == -1) exit(99);	// Ignore all other errors
		        memset(commandArray[i], 0, sizeof(commandArray[i])); // Copies char 0 (NULL) to the first n (size of userInput) characters of userInput (Basically clears the userInput)
            }
            continue;
        }
        // End of On-Liners

		errorCode = parseInput(userInput); // Calls parseInput function below
		if (errorCode == -1) exit(99);	// Ignore all other errors
		memset(userInput, 0, sizeof(userInput)); // Copies char 0 (NULL) to the first n (size of userInput) characters of userInput (Basically clears the userInput)
	}
	return 0;
}

// checks for the error code and calls interpreter function in interpreter.c
int parseInput(char ui[]) { // ui is userInput
    char tmp[200];
    char *words[100];                            
    int a = 0;
    int b;                            
    int w = 0; // wordID (number of words in userInput)  
    int errorCode;
    for(a = 0; ui[a] == ' ' && a < 1000; a++);        // Skip white spaces
    while(ui[a] != '\n' && ui[a] != '\0' && a < 1000) {
        for(b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' && ui[a] != ' ' && a < 1000; a++, b++) {
            tmp[b] = ui[a];                        
            // Extract a word
        }
        tmp[b] = '\0'; // Insert '\0' at the end of tmp
        words[w] = strdup(tmp); // Make duplicate and save it into words
        w++;
        if(ui[a] == '\0') break; // If the entire command (userInput) is finished, break from the while loop
        a++; 
    }
    errorCode = interpreter(words, w); // In interpreter.c
    return errorCode;
}