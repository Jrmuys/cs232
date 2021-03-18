//
//  commandline.h
//  CS232 Command Shell
//
//  Created by Victor Norman on 1/20/15.
//  Copyright (c) 2015 Victor Norman. All rights reserved.
//

#ifndef __CS232_Command_Shell__commandline__
#define __CS232_Command_Shell__commandline__

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class CommandLine
{
public:
    CommandLine(istream &in);
    char *getCommand() const { return argv[0]; }   
    int getArgCount() const { return argc; }
    char **getArgVector() const { return argv; }
    char *getArgVector(int i) const { return argv[i]; }
    bool noAmpersand() const { return ! ampersandSeen; }
    
    ~CommandLine();
private:
    void copyToArgv(vector<string> &tempArgv);

    int argc;
    char **argv;
    bool ampersandSeen;
};

#endif /* defined(__CS232_Command_Shell__commandline__) */
