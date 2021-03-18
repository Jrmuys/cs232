//
//  path.cpp
//  written for: CS232 Command Shell
//
//  Created by Joel Muyskens on 02/27/2021.
//
//  Path.cpp creates a vector of the directories found in the path variable
//  and provides a find(program) function to find where (or if) a program is
//  in any of the path directories.

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <dirent.h>
#include "path.h"

Path::Path()
{
    char *cPath = getenv("PATH"); // Get path variable
    string sPath = string(cPath); // convert to string

    size_t cPos;
    string delimiter = ":";
    string directory;

    // Finds position of colon, if it's not the end of the string, continue
    while ((cPos = sPath.find(delimiter)) != string::npos)
    {
        path.push_back(sPath.substr(0, cPos)); // Push portion before colon to path
        sPath.erase(0, cPos + 1);              // Erase that portion (including colon)
    }
}

int Path::find(const string &program) const
{
    int i = 0;
    for (string dir : path) // Iterate trough path vector
    {
        DIR *directory = opendir(dir.c_str()); // Open the directory
        dirent *entry;
        while (entry = readdir(directory)) // While there is an entry to look at
        {
            string d_name = entry->d_name; // Find the name of the program at that entry
            if (d_name == program)         // If the program has the same name as program, we found it...
            {
                return i; // ... so return the index it was found at
            }
        }
        closedir(directory); // Done looking through directory
        i++;
    }
    return -1;
}

Path::~Path()
{
    path.clear();
}