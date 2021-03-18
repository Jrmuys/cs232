#ifndef __CS232_Command_Shell__Path__
#define __CS232_Command_Shell__Path__

#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Path
{
public:
    Path();
    ~Path();
    int find(const string &program) const;
    string getDirectory(int i) const { return path[i]; };

private:
    vector<string> path;
};

#endif /* defined(__CS232_Command_Shell__Path) */
