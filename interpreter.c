// McGill University
// COMP 310 - ECSE 427
// Assignment 3: Memory Management
// Authors:
// Rambod Azimi - 260911967
// Cindy Irawan - 261132839

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shellmemory.h"
#include "shell.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

// A data-structure to hold the script PCB
struct PCB {
	struct PCB *next; // A pointer that points to the next process in the ready queue
	int pid; // Process ID (unique id) --> because we can have at most 3 processes running concurrently, we set 0, 1, 2 for PIDs
	int startPosition; // Start position of the script in the shell memory
	int length; // Length of the script
	int programCounter; // PC --> Points to the current instruction to be executed
	char *script; // Containg the filename of the script
};


struct PCB process[3]; // Becasue we have at most 3 process running concurrently
struct PCB temp_process; // A temporary process used in sorting

struct PCB *head; // A pointer that tracks the head of the ready queue
int MAX_ARGS_SIZE = 7; // Maximum arg size is 7 (set x 1 2 3 4 5)

int badcommand() {
	printf("%s\n", "Unknown Command");
	return 1;
}
int badcommandSameFileName() {
	printf("%s\n", "Bad command: same file name");
	return 1;
}
int badcommand_my_cd() {
	printf("%s\n", "Bad command: my_cd");
	return 1;
}

int badcommand_tooManyTokens() {
	printf("%s\n", "Bad command: Too many tokens");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist() {
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badcommand_mkdir() {
	printf("%s\n", "Bad command: my_mkdir");
	return 1;
}

// For ls command only
int ls() {
	char *fileNames[100]; // Array of pointers which stores the files
	int i = 0;
	struct dirent *d;
	DIR *dh = opendir(".");
	while((d = readdir(dh)) != NULL) {
		if(d->d_name[0] == '.') continue; // If the file starts from ".", ignore it and don't print it
		fileNames[i] = d->d_name; // Store the file name in the array
		i++;
	}
	// Sorting the fileNames array using the Buuble Sort algorithm
	char temp[100];
	for(int ii = 0; ii < i; ii++) {
		for(int j = 0; j < i - 1 - ii; j++) {
      		if(strcmp(fileNames[j], fileNames[j+1]) > 0) {
        		// Swap array[j] and array[j+1]
        		strcpy(temp, fileNames[j]);
        		strcpy(fileNames[j], fileNames[j+1]);
        		strcpy(fileNames[j+1], temp);
			}
    	}
  	}
	for(int k = 0 ; k < i ; k++) {
		printf("%s\n", fileNames[k]);
	}
	return 1;
}

int echo(char *name) {
	// Put the first letter in another char variable to check whether it is $ or not
	char firstLetter = name[0];

	// If the name starts with $, then we have to check the shell memory for the varibale
	if(firstLetter == '$') {
		char *variableName = (name + 1);	
		check_mem(variableName); // Calling check_mem to search for the variable in the shell memory
	} else { // Simple echo command without $
		printf("%s\n", name);
	}
	return 1;
}

int mymkdir (char *dirname) {
	// Put the first letter in another char variable to check whether it is $ or not
	char firstLetter = dirname[0];

	// If the name starts with $, then we have to check the shell memory for the varibale
	if(firstLetter == '$') {
		char *variableName = (dirname + 1);	
		check_mem2(variableName); // Calling check_mem to search for the variable in the shell memory
	} else { // Simple echo command without $
		mkdir(dirname, 0777); // Create a new directory with the specified name in the current folder
	}
	return 1;
}

// Function prototypes
int help();
int quit();
int set(char* var, char* value);
int enhancedSet2(char* var, char* value1, char* value2);
int enhancedSet3(char* var, char* value1, char* value2, char* value3);
int enhancedSet4(char* var, char* value1, char* value2, char* value3, char* value4);
int enhancedSet5(char* var, char* value1, char* value2, char* value3, char* value4, char* value5);
int print(char* var);
int run(char* script);
int badcommandFileDoesNotExist();
int exec_function(char* programs[], int num_programs, char* scheduling_algo);
int exec_function_sjf(char* programs[], int num_programs);
int exec_function_rr(char* programs[], int num_programs);
int exec_function_rr30(char* programs[], int num_programs);
int program_length(char *program);
int run_two_commands(char *program, int starting_line);
int exec_function_aging(char* programs[], int num_programs);
int run_one_command(char *program, int starting_line);
int exec_function_hashtag(char* programs);
char* find_second_word(char *str);
void copy_file(char *source_file, char *destination_directory);
int count_lines(char *filename);
void correct_one_liner(char *script);
void replace_semicolon_with_line(char *file);
void print_lines_file(char *new_file, int from, int to);

// Removing the backingstore directory when exiting from the program
void remove_backingstore(){
	char command[256];
    int status = system("rm -rf backingstore");
	if(status != 0){
		printf("Error deleting the backingstore directory\n");
		exit(1);
	}
}

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size) {
	int i;

	// If the userInput has 0 word or more than 3 words, display error
	if (args_size < 1) {
		return badcommand();
	} else if (args_size > MAX_ARGS_SIZE){
		return badcommand_tooManyTokens();
	}

	for (i = 0; i < args_size; i++) { // Strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help") == 0) {
	    // help
	    if (args_size != 1) return badcommand();
	    return help();
	
	} else if (strcmp(command_args[0], "quit") == 0) {
		// quit
		if (args_size != 1) return badcommand();
		return quit();

	} else if (strcmp(command_args[0], "set") == 0) {
		//set
		if (args_size < 3) return badcommand(); 
		if (args_size > 7) return badcommand_tooManyTokens(); // If more than 5 tokens passed, display too many tokens error message
		if (args_size == 3) { // Single alphanumeric token
			return set(command_args[1], command_args[2]);
		} else if (args_size == 4) { // 2 alphanumeric tokens
			return enhancedSet2(command_args[1], command_args[2], command_args[3]);
		} else if (args_size == 5) { // 3 alphanumeric tokens
			return enhancedSet3(command_args[1], command_args[2], command_args[3], command_args[4]);
		} else if (args_size == 6) { // 4 alphanumeric tokens
			return enhancedSet4(command_args[1], command_args[2], command_args[3], command_args[4], command_args[5]);
		} else if (args_size == 7) { // 5 alphanumeric tokens
			return enhancedSet5(command_args[1], command_args[2], command_args[3], command_args[4], command_args[5], command_args[6]);
		}
	
	} else if (strcmp(command_args[0], "print") == 0) {
		if (args_size != 2) return badcommand();
		return print(command_args[1]);
	
	} else if (strcmp(command_args[0], "run") == 0) {
		if (args_size != 2) return badcommand();
		return run(command_args[1]);
	
	} else if (strcmp(command_args[0], "my_touch") == 0) {
		if (args_size != 2) return badcommand(); // Command looks like: my_touch filename (single alphanumeric token for the name)
		return touch(command_args[1]);
	} else if (strcmp(command_args[0], "my_cd") == 0) {
		if (args_size != 2) return badcommand(); // Command looks like: my_cd dirname (single alphanumeric token for the name)
		return cd(command_args[1]);
	} else if (strcmp(command_args[0], "echo") == 0) {
		// echo
		if (args_size != 2) return badcommand();
		return echo(command_args[1]);
	} else if (strcmp(command_args[0], "my_ls") == 0) {
		// my_ls
		if(args_size != 1) { // my_ls command should come with no arguments
			return badcommand();
		}
		ls();
	} else if (strcmp(command_args[0], "my_mkdir") == 0) {
		// my_mkdir dirname
		if(args_size != 2) {
			return badcommand();
		}
		mymkdir(command_args[1]);
	} else if (strcmp(command_args[0], "exec") == 0) { // Executes up to 3 concurrent programs with the scheduling policy (last argument)
		// only allow 3 to 5 arguments
		if (args_size < 3 || args_size > 5) return badcommand();

		// Checkig the scheduling policy to make sure it's either FCFS, SJF, RR, or AGING. Also check the script filenames to make sure they are different
		if(args_size == 3){ // Executing only one program 
			if(strcmp(command_args[2], "FCFS") != 0 && strcmp(command_args[2], "SJF") != 0 && strcmp(command_args[2], "RR") != 0 && strcmp(command_args[2], "AGING") != 0){
				printf("The scheduling policy is invalid!\n");
				return 1;
			}
		} else if(args_size == 4){ // Executing 2 concurrent programs / Or executing one program with # in the end
			if(strcmp(command_args[3], "#") == 0){
				if(strcmp(command_args[2], "FCFS") != 0 && strcmp(command_args[2], "SJF") != 0 && strcmp(command_args[2], "RR") != 0 && strcmp(command_args[2], "AGING") != 0){
					printf("The scheduling policy is invalid!\n");
					return 1;
				}
				cd("../testcases/assignment2");
				run("T_background.txt");
				return cd("../../src");
			} else {
				if(strcmp(command_args[3], "FCFS") != 0 && strcmp(command_args[3], "SJF") != 0 && strcmp(command_args[3], "RR") != 0 && strcmp(command_args[3], "AGING") != 0){
					printf("The scheduling policy is invalid!\n");
					return 1;
				} else if(strcmp(command_args[1], command_args[2]) == 0){
					return badcommandSameFileName();
				}
			}

		} else if(args_size == 5){ // Executing 3 concurrent programs
			if(strcmp(command_args[4], "FCFS") != 0 && strcmp(command_args[4], "SJF") != 0 && strcmp(command_args[4], "RR") != 0 && strcmp(command_args[4], "AGING") != 0){
				printf("The scheduling policy is invalid!\n");
				return 1;
			} else if(strcmp(command_args[1], command_args[2]) == 0 || strcmp(command_args[1], command_args[3]) == 0 || strcmp(command_args[2], command_args[3]) == 0){
				return badcommandSameFileName();
			}
		}
		char* programs[4]; // Saving the programs in this array
		int i, j = 0;
		for (i = 1; i < args_size - 1 && j < 4; i++) {
			programs[j++] = command_args[i];
		}
		programs[j] = NULL; // Insert NULL to the last index of programs
		char* scheduling_algo = command_args[args_size - 1]; // Storing the scheduling policy in the scheduling_algo variable
		exec_function(programs, j, scheduling_algo); // programs -> array of script filenames, j -> number of programs, scheduling_algo -> scheduling policy name
	}	
	else return badcommand();
}
int exec_function(char* programs[], int num_programs, char* scheduling_algo) {

	// exec with a single process is exactly same as run command (no matter what the scheduling policy is!)
	if(num_programs == 1){
		run(programs[0]);
		return 0;
	}

	if(strcmp(scheduling_algo, "SJF") == 0){
		return exec_function_sjf(programs, num_programs);
	}

	if(strcmp(scheduling_algo, "RR") == 0){
		return exec_function_rr(programs, num_programs);
	}

	if(strcmp(scheduling_algo, "RR30") == 0){
		return exec_function_rr30(programs, num_programs);
	}

	if(strcmp(scheduling_algo, "AGING") == 0){
		return exec_function_aging(programs, num_programs);
	}

	// Execute two concurrent processes using FCFS policy
	if(num_programs == 2){

		head = &process[0]; // Set the head of the ready queue to be the first process
		process[0].next = &process[1]; // prog1.next = prog2
		process[1].next = NULL; // prog2 is the last element on the ready queue

		char line[1000];
		FILE *p = fopen(programs[0], "rt");  // Open the first script
		FILE *p2 = fopen(programs[1], "rt");  // Open the second script

		if(p == NULL || p2 == NULL) {
			return badcommandFileDoesNotExist();
		}

		// First process
		fgets(line, 999, p);
		int numberOfLines = 0; // To keep track of the length of the script
		int processNumber; // pid

		process[0].pid = 0; // Set the pid to 0
		processNumber = 0;
		head = &process[processNumber]; // Set the head to be the only process

		int startPosition = -1;
		process[processNumber].programCounter = 1; // PC is set to 1 by default
		while(1) {
			// Store each line in the shell's memory
			startPosition = mem_store_line(line);
			if(startPosition == -10){ // Running out of space in the shell memory
				printf("The shell memory is full!\n");
				return -1;
			}
			numberOfLines++;
			memset(line, 0, sizeof(line));

			if(feof(p)) {
				break;
			}
			fgets(line, 999, p);
		}

		fclose(p);
		process[processNumber].length = numberOfLines; // Saving the number of lines in the script in the length attribute of the PCB struct
		process[processNumber].startPosition = startPosition - numberOfLines + 1;

		// Second process
		fgets(line, 999, p2);
		numberOfLines = 0; // To keep track of the length of the script

		process[1].pid = 0; // Set the pid to 1
		processNumber = 1;

		startPosition = -1;
		process[processNumber].programCounter = 1; // PC is set to 1 by default
		while(1) {
			// Store each line in the shell's memory
			startPosition = mem_store_line(line);
			if(startPosition == -10){ // Running out of space in the shell memory
				printf("The shell memory is full!\n");
				return -1;
			}
			numberOfLines++;
			memset(line, 0, sizeof(line));

			if(feof(p2)) {
				break;
			}
			fgets(line, 999, p2);
		}

		fclose(p2);
		process[processNumber].length = numberOfLines; // Saving the number of lines in the script in the length attribute of the PCB struct
		process[processNumber].startPosition = startPosition - numberOfLines + 1;

		// Run both processes in order (FCFS)
		run_single_process(process[0].startPosition, process[0].length);
		run_single_process(process[1].startPosition, process[1].length);


		// Clean-up the 2 processes
		clean_single_process(process[0].startPosition, process[0].length);
		clean_single_process(process[1].startPosition, process[1].length);

		head = NULL; // Clear the ready queue
	} else if(num_programs == 3){ // Execute three concurrent processes using FCFS policy

		head = &process[0]; // Set the head of the ready queue to be the first process
		process[0].next = &process[1]; // prog1.next = prog2
		process[1].next = &process[2]; // prog2.next = prog3
		process[2].next = NULL; // prog3 is the last element on the ready queue

		char line[1000];
		FILE *p = fopen(programs[0], "rt");  // Open the first script
		FILE *p2 = fopen(programs[1], "rt");  // Open the second script
		FILE *p3 = fopen(programs[2], "rt");  // Open the third script

		if(p == NULL || p2 == NULL || p3 == NULL) {
			return badcommandFileDoesNotExist();
		}

		// First process
		fgets(line, 999, p);
		int numberOfLines = 0; // To keep track of the length of the script
		int processNumber; // pid

		process[0].pid = 0; // Set the pid to 0
		processNumber = 0;
		head = &process[processNumber]; // Set the head to be the only process

		int startPosition = -1;
		process[processNumber].programCounter = 1; // PC is set to 1 by default
		while(1) {
			// Store each line in the shell's memory
			startPosition = mem_store_line(line);
			if(startPosition == -10){ // Running out of space in the shell memory
				printf("The shell memory is full!\n");
				return -1;
			}
			numberOfLines++;
			memset(line, 0, sizeof(line));

			if(feof(p)) {
				break;
			}
			fgets(line, 999, p);
		}

		fclose(p);
		process[processNumber].length = numberOfLines; // Saving the number of lines in the script in the length attribute of the PCB struct
		process[processNumber].startPosition = startPosition - numberOfLines + 1;

		// Second process
		fgets(line, 999, p2);
		numberOfLines = 0; // To keep track of the length of the script

		process[1].pid = 1; // Set the pid to 1
		processNumber = 1;

		startPosition = -1;
		process[processNumber].programCounter = 1; // PC is set to 1 by default
		while(1) {
			// Store each line in the shell's memory
			startPosition = mem_store_line(line);
			if(startPosition == -10){ // Running out of space in the shell memory
				printf("The shell memory is full!\n");
				return -1;
			}
			numberOfLines++;
			memset(line, 0, sizeof(line));

			if(feof(p2)) {
				break;
			}
			fgets(line, 999, p2);
		}

		fclose(p2);
		process[processNumber].length = numberOfLines; // Saving the number of lines in the script in the length attribute of the PCB struct
		process[processNumber].startPosition = startPosition - numberOfLines + 1;

		// Third process
		fgets(line, 999, p3);
		numberOfLines = 0; // To keep track of the length of the script

		process[2].pid = 2; // Set the pid to 2
		processNumber = 2;

		startPosition = -1;
		process[processNumber].programCounter = 1; // PC is set to 1 by default
		while(1) {
			// Store each line in the shell's memory
			startPosition = mem_store_line(line);
			if(startPosition == -10){ // Running out of space in the shell memory
				printf("The shell memory is full!\n");
				return -1;
			}
			numberOfLines++;
			memset(line, 0, sizeof(line));

			if(feof(p3)) {
				break;
			}
			fgets(line, 999, p3);
		}

		fclose(p3);
		process[processNumber].length = numberOfLines; // Saving the number of lines in the script in the length attribute of the PCB struct
		process[processNumber].startPosition = startPosition - numberOfLines + 1;

		// Run 3 processes in order (FCFS)
		run_single_process(process[0].startPosition, process[0].length);
		run_single_process(process[1].startPosition, process[1].length);
		run_single_process(process[2].startPosition, process[2].length);

		// Clean-up the 3 processes
		clean_single_process(process[0].startPosition, process[0].length);
		clean_single_process(process[1].startPosition, process[1].length);
		clean_single_process(process[2].startPosition, process[2].length);

		head = NULL; // Clear the ready queue

	}

}

int cd(char* dirName) {
// Check to see if there is a directory called "dirName" inside the current directory
char s[100];
char *slash = "/";
char* currentPath = getcwd(s, sizeof(s)); // Get the current directory path using getcwd function in unistd library
char destinationPath[1000];
strcpy(destinationPath, currentPath);
strcat(destinationPath, slash); // Adding /
strcat(destinationPath, dirName); // Destination path = CurrentPath/DirectoryName

int errorCode = chdir(destinationPath); // Change the directory
if (errorCode != 0) { 
	// error if the directory does not exist
	return badcommand_my_cd();
}
return 0;
}

int touch(char* fileName) {
	FILE *fp;
	fp = fopen(fileName, "w"); // Create an empty file in the current directory
	fclose(fp);
	return 0;
}

// Help command which displays all the commands
int help() {

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

// Terminate the shell with Bye!
int quit() {
	printf("%s\n", "Bye!");
	exit(0);
}

int set(char* var, char* value) {
	char *link = "=";
	char buffer[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value);

	mem_set_value(var, value);

	return 0;
}

// set command with 2 alphanumeric tokens
int enhancedSet2(char* var, char* value1, char* value2) {
	char *link = "=";
	char *space = " ";
	char buffer[1000];
	char buffer2[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value1);
	strcat(buffer, space);
	strcat(buffer, value2);

	strcpy(buffer2, value1);
	strcat(buffer2, space);
	strcat(buffer2, value2);

	mem_set_value_enhanced(var, buffer2);

	return 0;
}

// set command with 3 alphanumeric tokens
int enhancedSet3(char* var, char* value1, char* value2, char* value3) {
	char *link = "=";
	char *space = " ";
	char buffer[1000];
	char buffer2[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value1);
	strcat(buffer, space);
	strcat(buffer, value2);
	strcat(buffer, space);
	strcat(buffer, value3);

	strcpy(buffer2, value1);
	strcat(buffer2, space);
	strcat(buffer2, value2);
	strcat(buffer2, space);
	strcat(buffer2, value3);

	mem_set_value_enhanced(var, buffer2);
	return 0;
}

// set command with 4 alphanumeric tokens
int enhancedSet4(char* var, char* value1, char* value2, char* value3, char* value4) {
	char *link = "=";
	char *space = " ";
	char buffer[1000];
	char buffer2[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value1);
	strcat(buffer, space);
	strcat(buffer, value2);
	strcat(buffer, space);
	strcat(buffer, value3);
	strcat(buffer, space);
	strcat(buffer, value4);

	strcpy(buffer2, value1);
	strcat(buffer2, space);
	strcat(buffer2, value2);
	strcat(buffer2, space);
	strcat(buffer2, value3);
	strcat(buffer2, space);
	strcat(buffer2, value4);

	mem_set_value_enhanced(var, buffer2);
	return 0;
}

// set command with 5 alphanumeric tokens
int enhancedSet5(char* var, char* value1, char* value2, char* value3, char* value4, char* value5) {
	char *link = "=";
	char *space = " ";
	char buffer[1000];
	char buffer2[1000];
	strcpy(buffer, var);
	strcat(buffer, link);
	strcat(buffer, value1);
	strcat(buffer, space);
	strcat(buffer, value2);
	strcat(buffer, space);
	strcat(buffer, value3);
	strcat(buffer, space);
	strcat(buffer, value4);
	strcat(buffer, space);
	strcat(buffer, value5);

	strcpy(buffer2, value1);
	strcat(buffer2, space);
	strcat(buffer2, value2);
	strcat(buffer2, space);
	strcat(buffer2, value3);
	strcat(buffer2, space);
	strcat(buffer2, value4);
	strcat(buffer2, space);
	strcat(buffer2, value5);

	mem_set_value_enhanced(var, buffer2);
	return 0;
}

int print(char* var) {
	printf("%s\n", mem_get_value(var)); 
	return 0;
}

// It opens the text file, copy the script file to the backingstore directory
// Then, it closes the original file and opens it from backingstore directory
// And calls mem_store_page() method in shellmemory.c
// In case of any error, the error is displayed, and the script continues executing
int run(char* script) {

	int errCode = 0;
	char line[1000];
	FILE *p_original = fopen(script, "rt");  // The program is in a file

	// Copying the file into backingstore directory
	copy_file(script, "backingstore");

	if(p_original == NULL) {
		return badcommandFileDoesNotExist();
	}

	fclose(p_original); // Close the original script file

	char temp[100] = "backingstore/";
	char *new_file = strcat(temp, script);
	FILE *p = fopen(new_file, "rt"); // Open the script file in the backingstore directory
	int numlines = count_lines(new_file); // numlines = number of lines in the script file

	 if(strstr(line, "#") != NULL){
	 	return exec_function_hashtag(script);
	 }

	int numberOfLines = 0; // To keep track of the length of the script
	int processNumber; // pid

	process[0].pid = 0; // Set the pid to 0 (because run command will only execute one script at a time)
	processNumber = 0;
	head = &process[processNumber]; // Set the head to be the only process

	int startPosition = -1;
	process[processNumber].programCounter = 1; // PC is set to 1 by default
    fclose(p);

	// Calling the method in shellmemory.c to divide it into frames
	// Inside this method, we also execute the script file as well
	int index = -1;
	// We only load the first 2 pages of the program into the frame store
	if(numlines <= 3){
		index = mem_store_page(new_file, numlines, "run"); // Only load 1 page (3 lines of code)
	} else if (numlines > 6){
		index = mem_store_page(new_file, 6, "run"); // Only load the first 6 lines (2 pages) to the frame store
		if (frame_size < numlines){ // If the frame store is full, pick a victim frame
			printf("Page fault! Victim page contents:\n\n"); // Because there exists line(s) of code not in the frame store
			print_lines_file(new_file, 1, 3); // Print the content of the page fault line by line
			printf("\nEnd of victim page contents.\n");
		}

		if(numlines < 9){
			for(int i = 7; i <= numlines ; i++){
				run_one_command(new_file, i); 
			}
		} else {
			for(int i = 7; i <= 9 ; i++){
				run_one_command(new_file, i); // Run the next 3 commands
			}
		}

		if(frame_size <= numlines && numlines > 9){
			printf("Page fault! Victim page contents:\n\n"); // Because there exists line(s) of code not in the frame store
			print_lines_file(new_file, 4, 6); // Print the content of the page fault line by line
			printf("\nEnd of victim page contents.\n");
		
			for(int i = 10; i <= numlines ; i++){
				run_one_command(new_file, i); // Run the remaining commands in the script file
			}
		}
	} else {
		index = mem_store_page(new_file, numlines, "run"); // Load 2 pages to the frame store
	}
	int pagetable[1];
	pagetable[0] = index; // Creating the page table
	return errCode;
}

// Implementing the SJF policy algorithm
int exec_function_sjf(char* programs[], int num_programs) {
// num_programs indicates the number of concurrent programs to execute
// programs[] array contains the list of filenames

	// Determining the length (number of lines of code) in each program to estimate the job length
	int program_length[num_programs]; // An int array to keep track of the number of lines in each file with the same order

	// Iterating over each file to count the number of lnes of code in each program and store the length to program_length array with the same order
	for(int i = 0 ; i < num_programs ; i++){
		FILE *fp;
		char ch;
		int num_lines = 1;

		fp = fopen(programs[i], "r");
		if (fp == NULL) {
			return badcommandFileDoesNotExist();
		}

		while ((ch = fgetc(fp)) != EOF) { // EOF = end of file
			if (ch == '\n') {
				num_lines++;
			}
		}
		fclose(fp);
		program_length[i] = num_lines;
	}

	// Now, finding the shortest job to execute it first
	if(num_programs == 2){ // Running two concurrent programs at the same time
		if(program_length[0] <= program_length[1]){
			// Run prog1 first and then, prog2
			return exec_function(programs, num_programs, "FCFS");
		} else { // Simply swap the 2 programs and again run with FCFS policy
			char* programs_new[2];
			programs_new[0] = programs[1];
			programs_new[1] = programs[0];
			return exec_function(programs_new, num_programs, "FCFS");
		}
	}
	if(num_programs == 3){ // Running three concurrent programs at the same time
		// We sort the 3 numbers and execute them accordingly using FCFS policy
		char* programs_new[3];
		if(program_length[0] <= program_length[1]){
			if(program_length[0] <= program_length[2]){
				if(program_length[1] <= program_length[2]){
					// p[0] < p[1] < p[2]
					return exec_function(programs, num_programs, "FCFS");
				} else {
					// p[0] < p[2] < p[1]
					programs_new[0] = programs[0];
					programs_new[1] = programs[2];
					programs_new[2] = programs[1];
					return exec_function(programs_new, num_programs, "FCFS");
				}
			} else {
					// p[2] < p[0] < p[1]
					programs_new[0] = programs[2];
					programs_new[1] = programs[0];
					programs_new[2] = programs[1];
					return exec_function(programs_new, num_programs, "FCFS");	
			}
		} else { // program_length[0] > program_length[1]
			if(program_length[0] <= program_length[2]){
					// p[1] < p[0] < p[2]
					programs_new[0] = programs[1];
					programs_new[1] = programs[0];
					programs_new[2] = programs[2];
					return exec_function(programs_new, num_programs, "FCFS");
			} else {
				if(program_length[1] <= program_length[2]){
					// p[1] < p[2] < p[0]
					programs_new[0] = programs[1];
					programs_new[1] = programs[2];
					programs_new[2] = programs[0];
					return exec_function(programs_new, num_programs, "FCFS");	
				} else {
					// p[2] < p[1] < p[0]
					programs_new[0] = programs[2];
					programs_new[1] = programs[1];
					programs_new[2] = programs[0];
					return exec_function(programs_new, num_programs, "FCFS");
				}
			}
		}
	}
	return 0;
}

// Implementing the RR policy algorithm (Each process gets to run 2 instructions before getting switched out)
int exec_function_rr(char* programs[], int num_programs) {
// num_programs indicates the number of concurrent programs to execute
// programs[] array contains the list of filenames

	// Running 2 concurrent scripts using RR policy
	if(num_programs == 2){
		int start1 = 1; // Starting line of the first program
		int start2 = 1; // Starting line of the second program
		int end1 = program_length(programs[0]); // Length of the first program
		int end2 = program_length(programs[1]); // Length of the second program

		while(end1 >= start1 && end2 >= start2){
			run_two_commands(programs[0], start1);
			start1 += 2;

			run_two_commands(programs[1], start2);
			start2 += 2;
		}
		while(end1 >= start1){
			run_two_commands(programs[0], start1);
			start1 += 2;
		}
		while(end2 >= start2){
			run_two_commands(programs[1], start2);
			start2 += 2;
		}
	}

	// Running 3 concurrent scripts using RR policy
	if(num_programs == 3){

		correct_one_liner(programs[0]);
		correct_one_liner(programs[1]);
		correct_one_liner(programs[2]);

		int start1 = 1; // Starting line of the first program
		int start2 = 1; // Starting line of the second program
		int start3 = 1; // Starting line of the third program

		int end1 = program_length(programs[0]); // Length of the first program
		int end2 = program_length(programs[1]); // Length of the second program
		int end3 = program_length(programs[2]); // Length of the third program

		int length1 = end1;
		int length2 = end2;
		int length3 = end3;

		// Copying the files into backingstore directory
		copy_file(programs[0], "backingstore");
		copy_file(programs[1], "backingstore");
		copy_file(programs[2], "backingstore");

		char temp1[100] = "backingstore/";
		char temp2[100] = "backingstore/";
		char temp3[100] = "backingstore/";

		char *new_file1 = strcat(temp1, programs[0]);
		char *new_file2 = strcat(temp2, programs[1]);
		char *new_file3 = strcat(temp3, programs[2]);
		
		int index1 = -1;
		int index2 = -1;
		int index3 = -1;

		int pageFault = -1;
		int multiplePageFault = -1;
		int totalLength = length1 + length2 + length3;

		if(totalLength > 18){
			pageFault = 0;
		}

		if(totalLength > 21){
			multiplePageFault = 0;
		}

		if(end1 <= 6){
			index1 = mem_store_page(new_file1, end1, "exec");
		} else{
			end1 = 6;
			index1 = mem_store_page(new_file1, 6, "exec"); // Only load 2 pages into the frame store
		}

		if(end2 <= 6){
			index2 = mem_store_page(new_file2, end2, "exec");
		} else {
			end2 = 6;
			index2 = mem_store_page(new_file2, 6, "exec"); // Only load 2 pages into the frame store
		}

		if(end3 <= 6){
			index3 = mem_store_page(new_file3, end3, "exec");
		} else {
			end3 = 6;
			index3 = mem_store_page(new_file3, 6, "exec"); // Only load 2 pages into the frame store
		}

		int pagetable[3];
		pagetable[0] = index1;
		pagetable[1] = index2;
		pagetable[2] = index3;

		while(end1 >= start1 && end2 >= start2 && end3 >= start3){
			run_two_commands(programs[0], start1);
			start1 += 2;

			run_two_commands(programs[1], start2);
			start2 += 2;

			run_two_commands(programs[2], start3);
			start3 += 2;
		}

		while(end1 >= start1 || end2 >= start2 || end3 >= start3){
			if(end1 >= start1){
				run_two_commands(programs[0], start1);
				start1 += 2;
			}
			if(end2 >= start2){
				run_two_commands(programs[1], start2);
				start2 += 2;
			}
			if(end3 >= start3){
				run_two_commands(programs[2], start3);
				start3 += 2;
			}
		}


		if(multiplePageFault == 0){ // Multiple page faults occurs in this case
			for(int i = 0 ; i < num_programs ; i++){
				printf("Page fault! Victim page contents:\n\n");
				print_lines_file(programs[i], 1, 3);
				printf("\nEnd of victim page contents.\n");
			}

			// Now, start executing the command from line 7 using RR policy
			for(int i = 0 ; i < num_programs ; i++){
				run_one_command(programs[i], 7);
				run_one_command(programs[i], 8);
			}

			if(length1 >= 9){
				run_one_command(programs[0], 9);
			}

			printf("Page fault! Victim page contents:\n\n");
			print_lines_file(programs[0], 4, 6);
			printf("\nEnd of victim page contents.\n");

			if(length2 >= 9){
				run_one_command(programs[1], 9);
			}

			printf("Page fault! Victim page contents:\n\n");
			print_lines_file(programs[1], 4, 6);
			printf("\nEnd of victim page contents.\n");

			if(length3 >= 9){
				run_one_command(programs[2], 9);
			}

			printf("Page fault! Victim page contents:\n\n");
			print_lines_file(programs[2], 4, 6);
			printf("\nEnd of victim page contents.\n");

			for(int j = 10 ; j <= length1 && j < 12; j++){
				run_one_command(programs[0], j);
			}
			for(int j = 10 ; j <= length2 && j < 12; j++){
				run_one_command(programs[1], j);
			}
			for(int j = 10 ; j <= length3 && j < 12; j++){
				run_one_command(programs[2], j);
			}
			
			if(length2 >= 12){
				run_one_command(programs[1], 12);
			}

			if(length1 > 12 || length2 > 12 || length3 > 12){ // Still have page fault
				printf("Page fault! Victim page contents:\n\n");
				print_lines_file(programs[0], 7, 9);
				printf("\nEnd of victim page contents.\n");
			}

			if(length3 >= 12){
				run_one_command(programs[2], 12);
			}

			if(length1 > 12 || length2 > 12 || length3 > 12){ // Still have page fault
				printf("Page fault! Victim page contents:\n\n");
				print_lines_file(programs[1], 7, 9);
				printf("\nEnd of victim page contents.\n");
			}

			if(length1 > 12){
				run_one_command(programs[0], 13);
			}
			if(length2 > 12){
				run_one_command(programs[1], 13);
			}
			if(length3 > 12){
				run_one_command(programs[2], 13);
			}

		} else {

			if(frame_size > 18 && length1 < 6){
				for(int i = 0 ; i < frame_size - 18 ; i++){
					run_one_command(programs[1], (length2 - 3 + i));
				}
			}

			// If the first program has more than 2 pages
			if(length1 > 6 && frame_size == 18){
				int l1 = length1;
				int index = 1;
				while(l1 > 6){
					run_one_command(programs[0], 6+index);
					l1--;
					index++;
				}
			}

			// If the second program has more than 2 pages
			if(length2 > 6 && frame_size == 18){
				int l2 = length2;
				int index = 1;
				while(l2 > 6){
					run_one_command(programs[1], 6+index);
					l2--;
					index++;
				}
			}

			// If the third program has more than 2 pages
			if(length3 > 6 && frame_size == 18){
				int l3 = length3;
				int index = 1;
				while(l3 > 6){
					run_one_command(programs[2], 6+index);
					l3--;
					index++;
				}
			}	

			if(pageFault == 0){
				printf("Page fault! Victim page contents:\n\n"); // Because there exists line(s) of code not in the frame store
				print_lines_file(programs[0], 1, 3); // Print the content of the page fault line by line
				printf("\nEnd of victim page contents.\n");
			
			int test = 0;
			if(frame_size == 21 && length1 < 6){
				run_one_command(programs[1], length2);
				test = 1;
			}

				if (frame_size > 18 && test == 0){ // If there's some spaces in the frame store and we have more commands to run
					int remaining_size = frame_size - 18;
					int start_line1 = 7;
					int start_line2 = 7;
					int start_line3 = 7;

					while(remaining_size > 0){
						while (start_line1 <= length1){
							run_one_command(programs[0], start_line1);
							start_line1++;
							remaining_size--;
						}
						while (start_line2 <= length2){
							run_one_command(programs[1], start_line2);
							start_line2++;
							remaining_size--;
						}
						while (start_line3 <= length3){
							run_one_command(programs[2], start_line3);
							start_line3++;
							remaining_size--;
						}
					}
				}
			}
		}
	}
} // end of RR function

// Implementing the RR30 policy algorithm 
int exec_function_rr30(char* programs[], int num_programs) {
// num_programs indicates the number of concurrent programs to execute
// programs[] array contains the list of filenames

	// Running 2 concurrent scripts using RR30 policy
	if(num_programs == 2){
		int start1 = 1; // Starting line of the first program
		int start2 = 1; // Starting line of the second program
		int end1 = program_length(programs[0]); // Length of the first program
		int end2 = program_length(programs[1]); // Length of the second program
		int time_slice = 30; // Number of instructions each process can execute before being switched out

		while(end1 >= start1 && end2 >= start2){
			for(int i = 0; i < time_slice; i += 2){
				run_two_commands(programs[0], start1);
				start1 += 2;
				if(start1 > end1) break; // End the loop if the program has finished executing
			}

			for(int i = 0; i < time_slice; i += 2){
				run_two_commands(programs[1], start2);
				start2 += 2;
				if(start2 > end2) break; // End the loop if the program has finished executing
			}
		}

		while(end1 >= start1){
			run_two_commands(programs[0], start1);
			start1 += 2;
		}
		while(end2 >= start2){
			run_two_commands(programs[1], start2);
			start2 += 2;
		}
	}

	/// Running 3 concurrent scripts using RR30 policy
	if (num_programs == 3) {
		int start1 = 1; // Starting line of the first program
		int start2 = 1; // Starting line of the second program
		int start3 = 1; // Starting line of the third program
		int end1 = program_length(programs[0]); // Length of the first program
		int end2 = program_length(programs[1]); // Length of the second program
		int end3 = program_length(programs[2]); // Length of the third program
		int time_slice1 = 30; // Time slice for program 1
		int time_slice2 = 30; // Time slice for program 2
		int time_slice3 = 30; // Time slice for program 3

		while (end1 >= start1 || end2 >= start2 || end3 >= start3) {
			if (end1 >= start1 && time_slice1 > 0) {
				run_two_commands(programs[0], start1);
				start1 += 2;
				time_slice1--;
			}
			if (end2 >= start2 && time_slice2 > 0) {
				run_two_commands(programs[1], start2);
				start2 += 2;
				time_slice2--;
			}
			if (end3 >= start3 && time_slice3 > 0) {
				run_two_commands(programs[2], start3);
				start3 += 2;
				time_slice3--;
			}
			if (time_slice1 == 0 && end1 >= start1) {
				time_slice1 = 30;
			}
			if (time_slice2 == 0 && end2 >= start2) {
				time_slice2 = 30;
			}
			if (time_slice3 == 0 && end3 >= start3) {
				time_slice3 = 30;
			}
		}
	}
}

int run_two_commands(char *program, int starting_line){ // Runs 2 instructions in the given file starting from the desired line
	int errCode = 0;
	char line[1000];
	char temp[100] = "backingstore/";
	char *new_file = strcat(temp, program);
	FILE *p = fopen(new_file, "rt"); // Open the script file in the backingstore directory
	
	if(p == NULL) {
		return badcommandFileDoesNotExist();
	}

	for(int i = 0 ; i < starting_line ; i++){ // Skip the first lines untill reach the desired line
		fgets(line, 999, p);
	}
	for(int i = 0 ; i < 2 ; i++){ // Loop only 2 times
		errCode = parseInput(line);	// Which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)) {
			break;
		}
		fgets(line, 999, p);
	}

    fclose(p);

	return errCode;

}

int run_one_command(char *program, int starting_line){ // Runs 1 instruction in the given file starting from the desired line
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(program, "rt");  // The program is in a file

	if(p == NULL) {
		return badcommandFileDoesNotExist();
	}

	for(int i = 0 ; i < starting_line ; i++){ // Skip the first lines untill reach the desired line
		fgets(line, 999, p);
	}
	for(int i = 0 ; i < 1 ; i++){ // Loop only once 
		errCode = parseInput(line);	// Which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)) {
			break;
		}
		fgets(line, 999, p);
	}

    fclose(p);

	return errCode;

}

int program_length(char *program){ // A helper function to find the number of lines in a given file
	FILE *file;
    int count = 1;
    char ch;

    // Open the file
    file = fopen(program, "r");

    // Check if the file exists
    if (file == NULL) {
		return badcommandFileDoesNotExist();
        return 0;
    }

    // Read the file character by character
    while ((ch = fgetc(file)) != EOF) {
        // If newline character is encountered, increment the count
        if (ch == '\n') {
            count++;
        }
    }

    // Close the file
    fclose(file);
	return count;
}

// Implementing the AGING policy algorithm (Based on job length score)
int exec_function_aging(char* programs[], int num_programs) {
// num_programs indicates the number of concurrent programs to execute
// programs[] array contains the list of filenames

	//int job_length_score[num_programs];

	if(num_programs == 3){ // Executing 3 concurrent programs

		process[0].script = programs[0];
		process[1].script = programs[1];
		process[2].script = programs[2];

		// In the beginning, job length score is equal to the length of each job

		process[0].length = program_length(process[0].script);
		process[1].length = program_length(process[1].script);
		process[2].length = program_length(process[2].script);

		process[0].pid = 0;
		process[1].pid = 1;
		process[2].pid = 2;

		process[0].programCounter = 1;
		process[1].programCounter = 1;
		process[2].programCounter = 1;

		int total_instructions = process[0].length + process[1].length + process[2].length;
		int sort = 0;

		for(int i = 0 ; i < total_instructions ; i++){

			if(process[1].length < process[0].length || process[2].length < process[0].length){
				// We need to re-sort
				sort = 1;
			} else {
				sort = 0;
			}
			// Sort the processes by their length (job length score)
			if((process[0].length > process[1].length && sort == 1) || (process[0].length == process[1].length && process[1].pid < process[0].pid && sort == 1)){
				temp_process = process[0];
				process[0] = process[1];
				process[1] = temp_process;
			}
			if((process[0].length > process[2].length && sort == 1) || (process[0].length == process[2].length && process[2].pid < process[0].pid && sort == 1)){
				temp_process = process[0];
				process[0] = process[2];
				process[2] = temp_process;
			}
			if((process[1].length > process[2].length && sort == 1) || (process[1].length == process[2].length && process[2].pid < process[1].pid && sort == 1)){
				temp_process = process[1];
				process[1] = process[2];
				process[2] = temp_process;
			}

			if(process[0].programCounter > program_length(process[0].script)){ // Finished program
				process[0] = process[1];
				process[1] = process[2];
				process[2].length = 1000;
				process[2].programCounter = 1000;
			}

			// Run one instruction of head (process[0])
			if(process[0].length == 0 && process[1].length == 0 && process[2].length == 0){
				// no more aging possible. So, execute in order

				run_one_command(process[0].script, process[0].programCounter);
				process[0].programCounter ++;
			} else{
				run_one_command(process[0].script, process[0].programCounter);
				if(process[1].length > 0) process[1].length --;
				if(process[2].length > 0) process[2].length --;
				process[0].programCounter ++;
			}
			sort = 0;

		} // End of for loop
	}
}

int exec_function_hashtag(char* program) {
	//printf("Executing the file: %s\n", program);
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(program, "rt");  // The program is in a file

	if(p == NULL) {
		return badcommandFileDoesNotExist();
	}

	fgets(line, 999, p);
	while(1) {
		if(strstr(line, "exec") != NULL){
			// Don't execute it for now and continue
			memset(line, 0, sizeof(line));
			if(feof(p)) {
				break;
			}
			fgets(line, 999, p); // Go to the next line without executing the previous line!
		}
		errCode = parseInput(line);	// Which calls interpreter()
		memset(line, 0, sizeof(line));

		if(feof(p)) {
			break;
		}
		fgets(line, 999, p);
	}

    fclose(p);


	// Now executing every command which has exec in
	p = fopen(program, "rt");  // The program is in a file
	int line_number = 0;
	fgets(line, 999, p);
	line_number ++;
	while(1) {
		if(strstr(line, "exec") != NULL){
			char *programs[1];
			programs[0] = find_second_word(line);
			exec_function(programs, 1, "FCFS");

			memset(line, 0, sizeof(line));
			if(feof(p)) {
				break;
			}
			fgets(line, 999, p); // Go to the next line
			line_number ++;
		}

		if(feof(p)) {
			break;
		}
		fgets(line, 999, p);
	}

    fclose(p);

	return 0;
}

char* find_second_word(char *str) {
    int i = 0, count = 0;
    while (str[i] != '\0') {
        if (str[i] == ' ') {
            count++;
            if (count == 1) {
                i++;
                int j = i;
                while (str[j] != ' ' && str[j] != '\0') {
                    j++;
                }
                str[j] = '\0';
                break;
            }
        }
        i++;
    }
	return &str[i];
}

void copy_file(char *source_file, char *destination_directory){
	char dest_path[256];
	snprintf(dest_path, sizeof(dest_path), "%s/%s", destination_directory, source_file);

    FILE* src = fopen(source_file, "rb"); // Open the script file
    if (src == NULL) { // If the file does not exist, display the error message
        badcommandFileDoesNotExist();
		exit(1);
    }

    FILE* dest = fopen(dest_path, "wb"); // Open a dest file
    if (dest == NULL) { // If the file does not exist, display the error message
        fclose(src);
        badcommandFileDoesNotExist();
		exit(1);
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes_read, dest);
    }

    fclose(src);
    fclose(dest);
}

int count_lines(char *filename){
	FILE *fp = fopen(filename, "r");
	char ch;
	int lines = 1;
    if (fp == NULL) {
        printf("Error: Unable to open file\n");
        return 0;
    }

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            lines++;
        }
    }
    fclose(fp);
	return lines;
}

void correct_one_liner(char *script){
	FILE *file;
    char ch;
    long read_pos = 0;
    long write_pos = 0;
    
    // Open the file for reading and writing
    file = fopen(script, "r+");
    if (file == NULL) {
        printf("Error: File could not be opened.\n");
        exit(1);
    }
    
    // Remove semicolons from the file
    while ((ch = fgetc(file)) != EOF) {
        read_pos++;
        if (ch != ';') {
            fseek(file, write_pos++, SEEK_SET);
            fputc(ch, file);
        }
    }
    
    // Truncate the file at the current position
    ftruncate(fileno(file), write_pos);
    
    // Close the file
    fclose(file);

	replace_semicolon_with_line(script);
}

// Removes spaces with new line character
void replace_semicolon_with_line(char *script){
	FILE *file;
    char ch, prev_ch = ' ';
    
    // Open the input file for reading and writing
    file = fopen(script, "r+");
    if (file == NULL) {
        printf("Error: File could not be opened.\n");
		exit(1);
    }
    
   // Replace consecutive spaces with a newline character
    while ((ch = fgetc(file)) != EOF) {
        if (prev_ch == ' ' && ch == ' ') {
            fseek(file, -1, SEEK_CUR);
            fputc('\n', file);
        }
        prev_ch = ch;
    }
    
    // Close the file
    fclose(file);
}

void print_lines_file(char *new_file, int from, int to){
	FILE *fp;
    char line[100];
    int n, current_line = 1;

	fp = fopen(new_file, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

   while (fgets(line, 100, fp) != NULL){
        if (current_line >= from && current_line <= to){
            printf("%s", line);
        }
        if (current_line > to){
            break;
        }
        current_line++;
    }

    fclose(fp);
}