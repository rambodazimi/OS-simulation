# OS-simulation
OS Simulation supporting Shell commands, Memory management (demand paging), and Process Management (schedule)

## Phase 1: Building an OS Shell

### Compiling your starter shell

• Use the following command to compile: make mysh

• Re-compiling your shell after making modifications: make clean; make mysh


### Running the script

• Interactive mode: From the command line prompt type: ./mysh

• Batch mode: You can also use input files to run your shell. To use an input file, from the command line prompt type:

./mysh < testfile.txt


### COMMAND DESCRIPTION

• help Displays all the commands

• quit Exits / terminates the shell with “Bye!”

• set VAR STRING Assigns a value to shell memory

• print VAR Displays the STRING assigned to VAR

• run SCRIPT.TXT Executes the file SCRIPT.TXT

• echo Displays one token string

• my_ls Lists all the files present in the current directory

• my_mkdir dirname Creates a new directory in the current directory

• my_touch filename Creates a new empty file inside the current directory

• my_cd dirname Changes current directory to directory dirname


