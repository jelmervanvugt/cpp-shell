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