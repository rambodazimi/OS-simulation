// McGill University
// COMP 310 - ECSE 427
// Assignment 3: Memory Management
// Authors:
// Rambod Azimi - 260911967
// Cindy Irawan - 261132839

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "shell.h"

int store_to_frame_store(char *first, char *second, char *third);
void run_single_command(char *cmd);

struct memory_struct {
	char *var;
	char *value;
	char *command;
};

struct frame_memory {
	char *first_line;
	char *second_line;
	char *third_line;
};

struct memory_struct shellmemory[1000];
struct frame_memory frame_memory[960]; // The first 960 slots are reserved for frames
char *variable_memory[40];

// Helper functions
int match(char *model, char *var) {
	int i, len = strlen(var), matchCount = 0;
	for(i = 0; i < len; i++)
		if (*(model+i) == *(var+i)) matchCount++;
	if (matchCount == len)
		return 1;
	else
		return 0;
}

char *extract(char *model) {
	char token = '=';    // Look for this to find value
	char value[1000];  // Stores the extract value
	int i, j, len = strlen(model);
	for(i = 0; i < len && *(model+i) != token; i++); // Loop till we get there
	// Extract the value
	for(i = i+1, j = 0; i < len; i++, j++) value[j] =* (model+i);
	value[j] = '\0';
	return strdup(value);
}

// Shell memory functions
// Initializing the backing store for paging
void mem_init() {

	int i;
	// Initializing all the shellmemory to "none"
	for(i = 0; i < 1000; i++) {		
		shellmemory[i].var = "none";
		shellmemory[i].value = "none";
		shellmemory[i].command = "none";
	}

	// Creating a new directory
	char *dir = "backingstore";
    int status = mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(status == -1){ // backingstore directory already present
		// Remove all contents in the backingstore
		const char* command = "rm -rf backingstore/*"; // remove all files in the directory
		system(command);
	}

	// Framing memory init
	for(int i = 0 ; i < 960 ; i++){
		frame_memory[i].first_line = "none"; // Initializing the first 960 slots to "none" for frames
		frame_memory[i].second_line = "none";
		frame_memory[i].third_line = "none";
	}

	// Variable store init
	for(int i = 0 ; i < 40 ; i++){
		variable_memory[i] = "none"; // Initializing the 40 slots to "none" for variables
	}
}

// Checks the shell memory for the variable and returns the value saved in the varibale 
int check_mem(char *variableName) {
	int i;
	for (i = 0; i < 1000; i++) { // Check all the contents of shell memory one by one
		if (strcmp(shellmemory[i].var, variableName) == 0) { // Success
			printf("%s\n", shellmemory[i].value); // Print the value associated to the variable
			return 0;
		}
	}
	// Failure
	printf("\n"); // Print an empty line in case the variable does not exist in the shell memory
	return 1;
}

// Another version of check_mem function
int check_mem2(char *variableName) {
	int i;
	for (i = 0; i < 1000; i++) { // Check all the contents of shell memory one by one
		if (strcmp(shellmemory[i].var, variableName) == 0) { // Success
			mkdir(shellmemory[i].value, 0777); // Create a new directory with the specified name in the current folder
			return 0;
		}
	}
	// Failure
	printf("%s\n", "Bad command: my_mkdir");
	return 1;
}

// Set key value pair (single alphanumeric token)
void mem_set_value(char *var_in, char *value_in) {
	int i;
	for (i = 0; i < 1000; i++) {
		if (strcmp(shellmemory[i].var, var_in) == 0) {
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}
	// Value does not exist, need to find a free spot.
	for (i = 0; i < 1000; i++) {
		if (strcmp(shellmemory[i].var, "none") == 0) {
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}
	return;
}

// Enhanced version of mem_set_value function which supports values up to 5 alphanumeric tokens
void mem_set_value_enhanced(char *var_in, char value_in[]) {
	int i;
	for (i = 0; i < 1000; i++) {
		if (strcmp(shellmemory[i].var, var_in) == 0) {
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}
	// Value does not exist, need to find a free spot.
	for (i = 0; i < 1000; i++) {
		if (strcmp(shellmemory[i].var, "none") == 0) {
			shellmemory[i].var = strdup(var_in);
			shellmemory[i].value = strdup(value_in);
			return;
		} 
	}
	return;
}

// Get value based on input key
char *mem_get_value(char *var_in) {
	int i;

	for (i = 0; i < 1000; i++) {
		if (strcmp(shellmemory[i].var, var_in) == 0) {

			return strdup(shellmemory[i].value);
		} 
	}
	return "Variable does not exist";
}

// Storing each line of the file (a command) to the shell's memory
// It returns the starting position of the script in the shell's memory
int mem_store_line(char *cmd) {
	for(int i = 0 ; i < 1000 ; i++){
		// Finding a free spot to store each line there
		if(strcmp(shellmemory[i].command, "none") == 0){
			shellmemory[i].command = strdup(cmd);
			return i;
		}
	}
	return -10;
}

int mem_store_page(char *ff, int numlines, char *mode) {
	FILE *p = fopen(ff, "rt"); // Open the script file in the backingstore directory
	char first[1000];
	char second[1000];
	char third[1000];
	int index = -1;
	if(p == NULL) {
		printf("File not found!\n");
		exit(1);
	}

	while(numlines > 0){
		if(numlines >=3){
			fgets(first, 999, p); // Read the first line and store it in first variable
			fgets(second, 999, p); // Read the second line and store it in second variable
			fgets(third, 999, p); // Read the third line and store it in third variable
			numlines -= 3;
			index = store_to_frame_store(first, second, third);
			if(strcmp(mode, "run") == 0){
				run_single_command(first); // Run the commands line by line
				run_single_command(second);
				run_single_command(third);
			}
		}
		if(numlines == 2){ // If only 2 lines remain in the script file
			fgets(first, 999, p); // Read the first line and store it in first variable
			fgets(second, 999, p); // Read the second line and store it in second variable
			numlines -= 2;
			store_to_frame_store(first, second, "empty");
			if(strcmp(mode, "run") == 0){
				run_single_command(first); // Run the commands line by line
				run_single_command(second);
			}
		}
		if(numlines == 1){ // If only 1 line remains in the script file
			fgets(first, 999, p); // Read the first line and store it in first variable
			numlines --;
			store_to_frame_store(first, "empty", "empty");
			if(strcmp(mode, "run") == 0){
				run_single_command(first); // Run the command
			}
		}
	}

	fclose(p);

	return index;
}

// Storing the 3 lines of code into the frame store
int store_to_frame_store(char *first, char *second, char *third){
	for(int i = 0 ; i < 960 ; i++){ // Searching for a free spot in the frame store
		if(strcmp(frame_memory[i].first_line, "none") == 0){
			frame_memory[i].first_line = first;
			frame_memory[i].second_line = second;
			frame_memory[i].third_line = third;
			return i; // return the index
		}
	}
	return -1; // Failure
}

void run_single_command(char *cmd){
	parseInput(cmd);
}

// Runs a single process with given startPosition, and length
void run_single_process(int startPosition, int length){

	// Iterate over the length of the whole script
	for(int i = 0 ; i < length ; i++){
	// Interpret command by command and execute it
	parseInput(shellmemory[startPosition + i].command);
	}
}

void clean_single_process(int startPosition, int length){
	for(int i = 0 ; i < length ; i++){
		shellmemory[startPosition + i].command = "none";
	}
}