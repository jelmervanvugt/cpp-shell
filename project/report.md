# Shell Report

## Operating System | Assignment 1 | 03-10-2021

| Student name | Student number |
| ------------ | -------------- | 
| Jiankai Zheng | s1080484 |
| Jelmer van Vught | ? |

## Design

### Communication between processes
Communication between processes is done by using pipes. 

The ancestor process (the very first process) holds a `pipes` array containing all the pipes that will be made during executing the commands.

During each iteration a new pipe will be made before forking. This pipe will be made and assigned to `pipes[i]` (`i` is the current iteration number). After forking the child will also have access to `pipes[i]`. Each child process will redirect the `STDIN_FILENO` to `pipes[i-1][READ_END]` using `dup2`. The first child is an exception as the first child process has to read the user input. Besides redirecting the `STDIN_FILENO` also the `STDOUT_FILENO` will be redirected to `pipes[i][WRITE_END]` for all children processes except for the last child process. The last child process will have to output the command results to the terminal.

This is in essence how the communication between the pipes is done. One child process writes to the write end of the pipe and the next one will read it from the read end of that same pipe.

### Executing expression in the background
When an expression from user input ends with a `&`, the expression must be executed in the background. The implementation of this is very simple. A new child process is created using `fork` and in that child process the `executeCommands` function is executed. This will evaluate the expression in the "background" and the parent process will continue which in this case is prompting the user again for input. When the background process is done evaluating the expression, it will just output the result in the terminal.

### Reading input from file and writing to file
If the first command ends with `< [file]`, instead of using the user input as input it will open the specified file and read it and use that as input. 

This is done by executing this: `open(expression.inputFromFile.c_str(), O_RDONLY, S_IRUSR);`. This will return a positive number if opening the file succeeded. This is the file descriptor id. If opening the file succeeded the `STDIN_FILENO` will be redirected to the file by using `dup2`. If there is a next command the `STDOUT_FILENO` also has to be redirected to `pipes[i][WRITE_END]` so that the next command can read it from the read end of the pipe. Otherwise just let it output to the terminal.

For writing to a file, the idea is the exactly the same. For opening a file we do: `open(expression.outputToFile.c_str(), O_CREAT | O_WRONLY, 0777);`. This will also return a positive number if it succeeded. If it succeeded we will redirect the `STDOUT_FILENO` to the specified file using the returned number (file descriptor id). 

### Handling exit and cd
The `exit` and `cd` command have to be handled differently than the other commands. 

When an `exit` is discovered during executing the commands, the shell will immediately exit the shell and return to the normal shell.

When a `cd` is discovered during executing the commands, it will perform this using `chdir`. This will move the current working directory to the one specified in the arguments. The subsequents commands will not be executed anymore and the loop will stop.

### Error handling
The code has error handling in most places. For instance if performing a `chdir` fails (returns a negative number) it will let the user know through the terminal. If creating a child process was not successful the expression will not be evaluated further and an appropriate error message will be shown. These are some examples, there are more error handling throughout the code.