#ifndef __CS232_Command_Shell__
#define __CS232_Command_Shell__

#include <iostream>
#include <string>
#include "path.h"
#include "prompt.h"
#include "commandline.h"
#include "path.h"
using namespace std;

class Shell
{
public:
    Shell(void);
    void run();
    ~Shell(void);

private:
    Prompt prompt;
    Path path;
    char **argv;
    int argc;
    bool noAmpersand = true;
    void cd(const string newDir) const;
    void pwd();
    void exit();
};

#endif /* defined(__CS232_Command_Shell__) */
