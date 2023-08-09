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

## Phase 2: Multi-process Scheduling

• run SCRIPT Executes the commands in the file SCRIPT

• run Assumes that a file exists with the provided file name, in the current directory. It opens that text file and then sends each line one at a time to the interpreter. The interpreter treats each line of text as a command. At the end of the script, the file is closed, and the command line prompt is displayed once more. While the script executes, the command line prompt is not displayed. If an error occurs while executing the script due a command syntax error, then the error is displayed, and the script continues executing.

• exec prog1 prog2 prog3 POLICY Executes up to 3 concurrent programs, according to a given scheduling policy

• exec takes up to four arguments. The two calls below are also valid calls of exec:

o exec prog1 POLICY

o exec prog1 prog2 POLICY

• POLICY is always the last parameter of exec.

• POLICY can take the following three values: FCFS, SJF, RR, or AGING. If other arguments are given, the shell outputs an error message, and exec terminates, returning the command prompt to the user.

Exec behavior for single-process. The behavior of exec prog1 POLICY is the same as the behavior of run prog1, regardless of the policy value. Use this comparison as a sanity check.








