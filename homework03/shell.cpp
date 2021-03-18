//
//  shell.cpp
//  CS232 Command Shell
//
//  Created by Joel Muyskens on 02/27/2021.
//
//  Shell implements a shell with the following features
//      Continuously runs and loops
//      Prompts the user for input
//      Parses commands using CommandLine
//      Implents "cd", "pwd", and "exit" commands
//      Uses Path class to find and run other programs
//      If & is detected, runs the program in the background
//

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <sys/wait.h>
#include "shell.h"

// #define DEBUGME 1

Shell::Shell(void)
{
    prompt = Prompt();
    path = Path();
}

void Shell::run()
{
    while (1)
    {

        prompt.set();
        cout << prompt.get(); // Prompt user

        CommandLine commandline = CommandLine(cin); // Await user input, parse with commandline

        // Store parsed commandline variables
        argv = commandline.getArgVector();
        argc = commandline.getArgCount();
        noAmpersand = commandline.noAmpersand();

        if (!argc) // If there are no arguments, restart loop
            continue;

        // Process built in commands
        string command = argv[0];
        if (command == "cd")
        {
            string newDir = argv[1];
            cd(newDir);
        }
        else if (command == "pwd")
        {
            pwd();
        }
        else if (command == "exit")
        {
            exit();
        }
        else
        {
            // Command was not found in built in commands
            int index;
            if ((index = path.find(argv[0])) >= 0) // Is the command in PATH?
            {

                pid_t pid = fork(); // If so, fork
                if (pid == -1)      // pid of -1 indicates error in the fork
                {
                    cout << "Error creating fork";
                    sched_yield();
                    continue;
                }
                else if (pid > 0) // pid of >0 indicates it is the parent
                {
                    if (noAmpersand)
                    {
                        int status;
                        waitpid(pid, &status, 0); // Wait for child if no '&' is given
                    }
                    else
                    {
                        sched_yield(); // otherwise synchronize,
                        continue;      // and continue
                    }
                }
                else // If pid = 0, this is the child process, so execute the program
                {
                    string fullpath = path.getDirectory(index) + "/" + argv[0]; // get full path for the program
                    if (!noAmpersand)
                        cout << "\n";
                    execve(fullpath.c_str(), argv, NULL); // Execute the program on the child process
                }
            }
            else
            {
                cout << "Command: " << argv[0] << " not found\n";
            }
        }
    };
}

void Shell::cd(const string newDir) const
{

#if DEBUGME
    cout << "Shell: Changing directory with arg: " << newDir << '\n';
#endif

    int result = chdir(newDir.c_str()); // Change the directory

    if (result < 0) // Negative result means the change didn't work
    {
        cout << "Directory not found\n";
#if DEBUGME
        cout << "Shell: Could not change directories\n";
#endif
    }
}

void Shell::pwd()
{

#if DEBUGME
    cout << "Shell: Printing working directory\n";
#endif

    char *tcwd = getcwd(NULL, 0); // gets current working directory
    if (tcwd == NULL)
    {
        cout << "Could not execute \"cwd\"";
        return;
    }

#if DEBUGME
    cout << "Shell: Could not get working directory\n";
#endif

    cout << "Working directory: " << string(tcwd) << '\n'; // Print out current working directory
}

void Shell::exit()
{
    abort();
}

Shell::~Shell(void)
{
    delete &prompt;
    delete &path;
}