# Shell Report

## Operating System | Assignment 1 | 03-10-2021

| Student name | Student number |
| ------------ | -------------- | 
| Jiankai Zheng | s1080484 |
| Jelmer van Vugt | s1081716 |

## Design

In this chapter the implementation design of the shell is discussed.

### Communication between processes
All communication between processes which are executing tasks for this shell goes through pipes. 

The very first process - also known as the ancestor process - possesses an array called `pipes` containing all the pipes that are used while executing the shell commands.

During each iteration a new pipe is made before forking. This pipe will be made and assigned to `pipes[i]` (where `i` is the current iteration number). After forking the child will also have access to `pipes[i]`. Each child process will redirect the `STDIN_FILENO` to `pipes[i-1][READ_END]` using `dup2`. After completing this step the child will be able to receive input through the pipe connecting it with their parent process. 

The first child created is an exception to this rule, since it reads its data from the user input and not another process. Besides redirecting the `STDIN_FILENO` also the `STDOUT_FILENO` will be redirected to `pipes[i][WRITE_END]` for all children processes except for the last child process. The last child process will have to output the command results to the terminal.

This is in essence how the communication between the pipes is done. One child process writes to the write end of the pipe and the next one will read it from the read end of that same pipe.

### Executing expression in the background
Usually when the shell is executing a expression put in by the user it does not allow the user to execute another expression till the current one has been completed and optional output has been written to the shell. The only exception to this rule is when the expression put in by the user end with a `&`.  When the expreesion ends with this character it is executed in the background without disabling the user to input another command.

The implementation of these so called background expressions is very straightforward. A new child process is created using `fork` after which that same child process calls the `executeCommands` function. This will evaluate the expression in the "background" and the parent process will continue which in this case is prompting the user again for input. When the background process is done evaluating the expression, it will just output the result in the terminal.

### Reading from and writing to files
If the first command ends with `< [file]`, instead of using the user input as input it will open the specified file, read its input and use it as an argument for the command at the left side of the expression. 

This is done by executing the following system call: `open(expression.inputFromFile.c_str(), O_RDONLY, S_IRUSR);`. When `open` succeeded opening the specified file it will return the file descriptor ID, which is always positive. When it failed opening the file of the file does not exist it will return a negative number. If opening the file succeeded the `STDIN_FILENO` will be redirected to the file by using `dup2`. If there is a next command the `STDOUT_FILENO` also has to be redirected to `pipes[i][WRITE_END]` in order that the next command can read it from the read end of the pipe. Otherwise just let it output to the terminal.

For writing to a file, the idea is the exactly the same. For opening a file we execute: `open(expression.outputToFile.c_str(), O_CREAT | O_WRONLY, 0777);`. This will also return a positive number if it succeeded. If it succeeded we will redirect the `STDOUT_FILENO` to the specified file using the returned number (file descriptor ID). 

### Handling exit and cd
The `exit` and `cd` command have to be handled differently than the other commands. 

When an `exit` is read during the execution of the commands, the shell will immediately exit the shell and return to the normal shell.

When a `cd` is discovered during executing the commands it will call the `chdir` system call. This will change the current working directory to the one specified in the arguments. The subsequents commands will not be executed anymore and the loop will stop.

### Error handling
The code has error handling in most places. For instance: if performing a `chdir` fails (returns a negative number) it will let the user know through the terminal. If creating a child process was not successful the expression will not be evaluated further and an appropriate error message will be shown. These are some examples, there are more error handling throughout the code.

## Testing 
### Weaknesses in provided tests
The provided unit tests only tests happy paths. That is nice that executing chained command works as it should. However does it also work correctly if the expression is not syntactically correct, or if the command can not be executed correctly? What is then expected? A proper error message? This should also be tested. 

To improve the test coverage we should extend it to also test exceptional paths, as discussed above.

### Common issues
By how interprocess communication is implemented the infinite buffer problem is not an issue in our shell. This is due the fact that the output is written to pipe and then read from that same pipe. In that process no intermediate storage is used to store that output. This circumvents the problem that the data stream could be potentially infinite.

Opened file descriptors are also guaranteed to be closed after they are not used anymore. This is done by the fact that they are immediately closed in the child process after the `STDIN_FILENO` or `STDOUT_FILENO` is redirected to the relevant end of the pipe. At the end of the loop the `pipe[i]` that is associated with that iteration is also guaranteed to be closed in the ancestor process. This guarantees that all file descriptors are closed after they are not needed anymore.
