/**
	* Shell
	* Operating Systems
	* v20.08.28
	*/

/**
	Hint: Control-click on a functionname to go to the definition
	Hint: Ctrl-space to auto complete functions and variables
	*/

// function/class definitions you are going to use
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <signal.h>
#include <string.h>

#include <vector>

// although it is good habit, you don't have to type 'std' before many objects by including this line
using namespace std;

struct Command
{
	vector<string> parts = {};
};

struct Expression
{
	vector<Command> commands;
	string inputFromFile;
	string outputToFile;
	bool background = false;
};

// Parses a string to form a vector of arguments. The seperator is a space char (' ').
vector<string> splitString(const string &str, char delimiter = ' ')
{
	vector<string> retval;
	for (size_t pos = 0; pos < str.length();)
	{
		// look for the next space
		size_t found = str.find(delimiter, pos);
		// if no space was found, this is the last word
		if (found == string::npos)
		{
			retval.push_back(str.substr(pos));
			break;
		}
		// filter out consequetive spaces
		if (found != pos)
			retval.push_back(str.substr(pos, found - pos));
		pos = found + 1;
	}
	return retval;
}

// wrapper around the C execvp so it can be called with C++ strings (easier to work with)
// always start with the command itself
// always terminate with a NULL pointer
// DO NOT CHANGE THIS FUNCTION UNDER ANY CIRCUMSTANCE
int execvp(const vector<string> &args)
{
	// build argument list
	const char **c_args = new const char *[args.size() + 1];
	for (size_t i = 0; i < args.size(); ++i)
	{
		c_args[i] = args[i].c_str();
	}
	c_args[args.size()] = nullptr;
	// replace current process with new process as specified
	::execvp(c_args[0], const_cast<char **>(c_args));
	// if we got this far, there must be an error
	int retval = errno;
	// in case of failure, clean up memory
	delete[] c_args;
	return retval;
}

// Executes a command with arguments. In case of failure, returns error code.
int executeCommand(const Command &cmd)
{
	auto &parts = cmd.parts;
	if (parts.size() == 0)
		return EINVAL;

	// execute external commands
	int retval = execvp(parts);

	return retval;
}

void displayPrompt()
{
	char buffer[512];
	char *dir = getcwd(buffer, sizeof(buffer));
	if (dir)
	{
		cout << "\e[32m" << dir << "\e[39m"; // the strings starting with '\e' are escape codes, that the terminal application interpets in this case as "set color to green"/"set color to default"
	}
	cout << "$ ";
	flush(cout);
}

string requestCommandLine(bool showPrompt)
{
	if (showPrompt)
	{
		displayPrompt();
	}
	string retval;
	getline(cin, retval);
	return retval;
}

// note: For such a simple shell, there is little need for a full blown parser (as in an LL or LR capable parser).
// Here, the user input can be parsed using the following approach.
// First, divide the input into the distinct commands (as they can be chained, separated by `|`).
// Next, these commands are parsed separately. The first command is checked for the `<` operator, and the last command for the `>` operator.
Expression parseCommandLine(string commandLine)
{
	Expression expression;
	vector<string> commands = splitString(commandLine, '|');
	for (size_t i = 0; i < commands.size(); ++i)
	{
		string &line = commands[i];
		vector<string> args = splitString(line, ' ');
		if (i == commands.size() - 1 && args.size() > 1 && args[args.size() - 1] == "&")
		{
			expression.background = true;
			args.resize(args.size() - 1);
		}
		if (i == commands.size() - 1 && args.size() > 2 && args[args.size() - 2] == ">")
		{
			expression.outputToFile = args[args.size() - 1];
			args.resize(args.size() - 2);
		}
		if (i == 0 && args.size() > 2 && args[args.size() - 2] == "<")
		{
			expression.inputFromFile = args[args.size() - 1];
			args.resize(args.size() - 2);
		}
		expression.commands.push_back({args});
	}
	return expression;
}

int executeCommands(Expression &expression)
{
	int pipes[expression.commands.size() - 1][2] = {};
	int READ_END = 0;
	int WRITE_END = 1;

	int outputFileId = -999;
	int inputFileId = -999;

	if (expression.outputToFile != "")
	{
		outputFileId = open(expression.outputToFile.c_str(), O_CREAT | O_WRONLY, 0777);
		if (outputFileId < 0)
		{
			cout << "Opening file failed." << endl;
		}
	}

	if (expression.inputFromFile != "")
	{
		inputFileId = open(expression.inputFromFile.c_str(), O_RDONLY, S_IRUSR);
		if (inputFileId < 0)
		{
			cout << "Opening file failed, or does not exist." << endl;
		}
	}

	for (int i = 0; i < expression.commands.size(); i++)
	{
		bool isFirst = i == 0;
		bool isLast = i == expression.commands.size() - 1;

		if (expression.commands[i].parts[0] == "exit")
		{
			//exits if an exit is found.
			cout << "Program exited succesfully!\n";
			exit(EXIT_SUCCESS);
		}
		else if (expression.commands[i].parts[0] == "cd")
		{

			int ch = chdir(expression.commands[i].parts[1].c_str());

			if (ch < 0)
			{
				cout << "Changing directory not succesfull\n";
			}
			
			//if cd is found then only execute that and ignore the other commands. we were allowed to decide how to handle this, and this was the easiest solution.
			break;
		}
		else
		{
			if (pipe(pipes[i]))
			{
				fprintf(stderr, "Pipe failed.\n");
				return EXIT_FAILURE;
			}

			pid_t child1 = fork();
			if (child1 == 0) //child
			{

				//whether a file input/output was handled. Handling file has their own flow, so if a file was handled, the normal flow is not necessary anymore.
				bool fileWasHandled = false;

				if (isFirst && inputFileId >= 0) //if is first command and has to read from file
				{
					dup2(inputFileId, STDIN_FILENO);
					if (expression.commands.size() > 1)
					{
						dup2(pipes[i][WRITE_END], STDOUT_FILENO); //if a next command has to executed then write to the pipe. otherwise just write to std out
					}
					close(pipes[i][READ_END]);
					close(pipes[i][WRITE_END]);
					close(inputFileId);
					fileWasHandled = true;
				}
				if (isLast && outputFileId >= 0) //if is last command and has to write to file
				{
					if (!isFirst) // it can be that a command is the first and last command (single command in the list), so we explicetly check whether it's not the first.
					{
						dup2(pipes[i - 1][READ_END], STDIN_FILENO); // if is not first then it has to read from pipe.
						close(pipes[i - 1][READ_END]);
						close(pipes[i - 1][WRITE_END]);
					}
					else
					{
						close(pipes[i][READ_END]);
						close(pipes[i][WRITE_END]);
					}

					dup2(outputFileId, STDOUT_FILENO);

					close(outputFileId);
					fileWasHandled = true;
				}

				if (!fileWasHandled) //if no file was handled, execute normal flow (no file input/output)
				{
					if (!isFirst)
					{
						close(pipes[i - 1][WRITE_END]);
						dup2(pipes[i - 1][READ_END], STDIN_FILENO);
						close(pipes[i - 1][READ_END]);
					}

					if (!isLast)
					{
						close(pipes[i][READ_END]);
						dup2(pipes[i][WRITE_END], STDOUT_FILENO);
						close(pipes[i][WRITE_END]);
					}
				}
				// free non used resources (why?)
				executeCommand(expression.commands[i]);
				abort(); // if the executable is not found, we should abort. (why?)
			}

			if (!isFirst)
			{
				close(pipes[i - 1][READ_END]);
				close(pipes[i - 1][WRITE_END]);
			}

			waitpid(child1, nullptr, 0);
		}
	}

	return 0;
}

int executeExpression(Expression &expression)
{
	// Check for empty expression
	if (expression.commands.size() == 0)
		return EINVAL;

	//if background is true then start the command processin in a different process so the parent process can just continue (with a new prompt in the shell for instance.)
	if (expression.background)
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			cout << "Executing commands in the background not possible. Cause: Creating of child process failed. Try again.";
		}
		else if (pid == 0) //do execute commands in a different process
		{
			return executeCommands(expression);
		}
	}
	else
	{
		//if background is false then just execute normally in the foreground
		return executeCommands(expression);
	}

	return 0;
}

int normal(bool showPrompt)
{
	while (cin.good())
	{
		string commandLine = requestCommandLine(showPrompt);
		Expression expression = parseCommandLine(commandLine);

		int rc = executeExpression(expression);
		if (rc != 0)
			cerr << strerror(rc) << endl;
	}
	return 0;
}

// framework for executing "date | tail -c 5" using raw commands
// two processes are created, and connected to each other
int step1(bool showPrompt)
{
	// create communication channel shared between the two processes
	// ...
	int mypipe[2];
	/* Create the pipe. */
	if (pipe(mypipe))
	{
		fprintf(stderr, "Pipe failed.\n");
		return EXIT_FAILURE;
	}

	pid_t child1 = fork();
	if (child1 == 0)
	{
		close(mypipe[0]);

		// redirect standard output (STDOUT_FILENO) to the input of the shared communication channel
		dup2(mypipe[1], STDOUT_FILENO);
		// free non used resources (why?)
		Command cmd = {{string("date")}};
		executeCommand(cmd);
		// display nice warning that the executable could not be found
		abort(); // if the executable is not found, we should abort. (why?)
	}

	pid_t child2 = fork();
	if (child2 == 0)
	{
		close(mypipe[1]);
		// redirect the output of the shared communication channel to the standard input (STDIN_FILENO).
		dup2(mypipe[0], STDIN_FILENO);
		// free non used resources (why?)
		Command cmd = {{string("tail"), string("-c"), string("5")}};
		executeCommand(cmd);
		abort(); // if the executable is not found, we should abort. (why?)
	}

	// free non used resources (why?)
	close(mypipe[0]);
	close(mypipe[1]);
	// wait on child processes to finish (why both?)
	waitpid(child1, nullptr, 0);
	waitpid(child2, nullptr, 0);
	return 0;
}

int shell(bool showPrompt)
{

	return normal(showPrompt);

	// return step1(showPrompt);
}
