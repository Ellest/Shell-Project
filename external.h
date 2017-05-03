#ifdef WINDOWS
    #include "stdafx.h"
#else
#endif
#include <unistd.h>
#include "command.h"

void checkPipesNumber(vector<string> argvc);
void threeCommands(const char * pathname, string sign[], char** cmd1, char** cmd2,char** cmd3,int cmd1Size,int cmd2Size,int cmd3Size);
void runExternal(const char * pathname, int cmdSize, char ** cstrCmd);
void pipeCommand(const char * pathname, char** cmd1, char** cmd2,int cmd1Size,int cmd2Size);
void redirectCommand(string sign, char** cmd1, char** cmd2,int cmd1Size,int cmd2Size);