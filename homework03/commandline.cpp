//
//  commandline.cpp
//  CS232 Command Shell
//
//  Created by Victor Norman on 1/20/15.
//  Copyright (c) 2015 Victor Norman. All rights reserved.
//
//  Commandline() edited by Joel Muyskens on 02/27/2021
//  For: homework 3
//
//  Purpose: check for "&" and push args to tempArgv vectory
//
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include "commandline.h"
using namespace std;

// #define DEBUGME 1

/**
 * Read in the command line from the user and parse it into argc and argv.
 */
CommandLine::CommandLine(istream &in)
{
    ampersandSeen = false;

    // Read whole line in from the user.
    string line;
    getline(in, line);

    // Make an istringstream which is useful for parsing line into words.
    istringstream iss(line);

    string word;
    // temporary vector of strings that we build up as we parse the cmd line.
    vector<string> tempArgv;
    while (getline(iss, word, ' '))
    {
#if DEBUGME
        cout << "CmdLine: read word " << word << endl;
#endif

        if (true)
        {
            if (word == "&") // filter out &
            {
                ampersandSeen = true;
            }
            // If word contains non space ' ' characters, then store it
            else if (word.find_first_not_of(' ') != string::npos ) 
            {
                
                tempArgv.push_back(word);
            }
        }
    }

#if DEBUGME
    cout << "CmdLine: done\n";
#endif

    // Copy the vector of strings to a C-style array of char *s.
    copyToArgv(tempArgv);
    tempArgv.clear();
}

void CommandLine::copyToArgv(vector<string> &tempArgv)
{
    argc = (int)tempArgv.size();

    // Malloc space for argv array of char pointers.  Make an extra one for the
    // NULL pointer terminator.
    argv = (char **)malloc(sizeof(char *) * (argc + 1));
    for (int i = 0; i < tempArgv.size(); i++)
    {
        argv[i] = (char *)malloc(tempArgv[i].size() + 1);
        strcpy(argv[i], tempArgv[i].c_str());
    }
    argv[argc] = NULL; // NULL-terminate the array.
}

CommandLine::~CommandLine()
{
    // free up the space for the argv.
    for (int i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    free(argv);
}
