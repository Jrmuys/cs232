//
//  prompt.cpp
//  CS232 Command Shell
//
//  Created by Victor Norman on 1/20/15.
//  Copyright (c) 2015 Victor Norman. All rights reserved.
//

#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "prompt.h"

Prompt::Prompt()
{
    set();
}

void Prompt::set()
{
    char *tcwd = getcwd(NULL, 0);
    cwd = string(tcwd);
    free(tcwd);
}
