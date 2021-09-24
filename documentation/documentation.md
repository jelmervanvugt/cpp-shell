# Documentatation Shell Assignment 

## Requirements

| Requirement | Notes | Finished? |
|-------------|-------|-----------|
| If command == exit, exit shell. | | |
| If command == cd and has one arg, change working dir of shell. | See man pages of chdir & execvp. | | 
| If input contains command: exec command and display output / error. Shells waits processing new input till command has finished completion. | | |
| If command is chained with \|, output of earlier command is used as input for the following. Only the output of the last command is displayed on screen, all error messages are directly printed on screen. Only the first command can process input from the user. | | | 
| If input ends with &: shell does not wait for completion of the command and executes in in background. The output and error messages will continue to be displayed on the screen. The first command can not get direct input from a user (if the input is not redirected from elsewhere this channel should be closed). | | |
| If last command ends with > (file), shell writes the output of last command to a file with the input as name. If the file exists it is overwritten. Of the last command only error messages are displayed on screen, no normal output. | | | 
| If the first command ends with < (file), executes input which is read from the given file in current dir. | | | 
| If commands cannot be executed, correct errormessages are given. | | | 
